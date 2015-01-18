// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: nadavs@google.com <Nadav Samet>
//         Jin Qing (http://blog.csdn.net/jq0123)

#include <iostream>
#include <boost/optional/optional.hpp>
#include <boost/thread/thread.hpp>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <zmq.hpp>

#include "rpcz/application.hpp"
#include "rpcz/connection.hpp"
#include "rpcz/connection_manager.hpp"
#include "rpcz/connection_manager_ptr.hpp"
#include "rpcz/replier.hpp"
#include "rpcz/rpc_channel.hpp"
#include "rpcz/server.hpp"
#include "sync_event.hpp"

#include "proto/search.pb.h"
#include "proto/search.rpcz.h"

using namespace std;

namespace rpcz {

class SearchServiceImpl : public SearchService {
 public:
  SearchServiceImpl() {
  }

  ~SearchServiceImpl() {
  }

  virtual void Search(
      const SearchRequest& request,
      replier replier_copy) {
    if (request.query() == "foo") {
      replier_copy.send_error(-4, "I don't like foo.");
    } else if (request.query() == "bar") {
      replier_copy.send_error(17, "I don't like bar.");
    } else if (request.query() == "timeout") {
      // We "lose" the request.
      return;
    } else {
      SearchResponse response;
      response.add_results("The search for " + request.query());
      response.add_results("is great");
      replier_copy.send(response);
    }
  }
};

class server_test : public ::testing::Test {
 public:
  server_test() :
      context_(new zmq::context_t(1)) /* scoped_ptr */ {
    EXPECT_TRUE(connection_manager::is_destroyed());
    application::set_zmq_context(context_.get());
    application::set_connection_manager_threads(10);
    connection_.reset(new connection);
    server_.reset(new server);
    start_server();
  }

  ~server_test() {
    // terminate the context, which will cause the thread to quit.
    application::terminate();
    server_.reset();
    service_.reset();
    connection_.reset();

    EXPECT_TRUE(connection_manager::is_destroyed());
    context_.reset();
  }

  void start_server() {
    rpcz::connection_manager_ptr cm = rpcz::connection_manager::get();
    service_.reset(new SearchServiceImpl);
    server_->register_singleton_service(*service_);
    server_->bind("inproc://myserver.frontend");
    *connection_ = cm->connect("inproc://myserver.frontend");
  }

protected:
  // destruct in reversed order
  scoped_ptr<zmq::context_t> context_;  // destruct last
  scoped_ptr<connection> connection_;
  scoped_ptr<SearchServiceImpl> service_;
  // Server must destruct before service. (Or unregister services before destruct.)
  scoped_ptr<server> server_;
};

struct handler {
  ::sync_event sync;
  SearchResponse response;
  boost::optional<rpc_error> error;

  SearchService_Stub::Search_Handler get_search_handler() {
    return boost::bind(&handler::on_response, this, _1);
  }
  error_handler get_error_handler() {
    return boost::bind(&handler::on_error, this, _1);
  }

 private:
  void on_response(const SearchResponse& resp)
  {
    response = resp;
    sync.signal();
  }
  void on_error(const rpc_error& err)
  {
    error.reset(err);
    sync.signal();
  }
};

/*
Test all kinds of request interfaces.
. Sync or async
. Explicit deadline or implicit default deadline
. Return response or use output parameter (only for sync)
. Explicit error handler or implicit default error handler (only for async)
There are 4 sync interfaces and 4 async interfaces.

Ordered as the declaration in search.rpcz.h.

XXX Add async interfaces without handler for oneway call.
*/

// Async interfaces:

TEST_F(server_test, AsyncRequestWithTimeout) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("timeout");
  handler hdl;
  stub.async_Search(request,
      hdl.get_search_handler(),
      hdl.get_error_handler(),
      1/*ms*/);
  hdl.sync.wait();
  ASSERT_TRUE(hdl.error);
  ASSERT_EQ(rpc_response_header::DEADLINE_EXCEEDED, hdl.error->get_status());
}

TEST_F(server_test, AsyncRequest) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("stone");
  handler hdl;
  stub.async_Search(request,
      hdl.get_search_handler(),
      hdl.get_error_handler());
  hdl.sync.wait();

  ASSERT_FALSE(hdl.error);
  ASSERT_EQ(2, hdl.response.results_size());
  ASSERT_EQ("The search for stone", hdl.response.results(0));
}

// Sync interfaces:

TEST_F(server_test, SyncRequest) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("stupid");
  SearchResponse response = stub.Search(request);
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for stupid", response.results(0));
}

TEST_F(server_test, RequestWithError) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("foo");
  try {
    (void)stub.Search(request);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(rpc_response_header::APPLICATION_ERROR, error.get_status());
    ASSERT_EQ("I don't like foo.", error.get_error_message());
  }
}

TEST_F(server_test, RequestWithTimeout) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("timeout");
  try {
    (void)stub.Search(request, 1/*ms*/);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(rpc_response_header::DEADLINE_EXCEEDED, error.get_status());
  }
}

TEST_F(server_test, RequestRaisesExceptions) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("foo");
  try {
    (void)stub.Search(request);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(status::APPLICATION_ERROR, error.get_status());
    ASSERT_EQ(-4, error.get_application_error_code());
  }
}

// Other interfaces:

TEST_F(server_test, SetDefaulDeadlineMs) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  SearchResponse response;
  request.set_query("timeout");
  stub.set_default_deadline_ms(1);
  try {
    stub.Search(request, &response);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(status::DEADLINE_EXCEEDED, error.get_status());
    return;
  }
  ASSERT_TRUE(false);
}

}  // namespace
