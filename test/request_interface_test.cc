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
#include <boost/thread/thread.hpp>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <zmq.hpp>

#include "rpcz/application.hpp"
#include "rpcz/callback.hpp"
#include "rpcz/connection.hpp"
#include "rpcz/connection_manager.hpp"
#include "rpcz/connection_manager_ptr.hpp"
#include "rpcz/replier.hpp"
#include "rpcz/rpc_channel.hpp"
#include "rpcz/rpc_controller.hpp"
#include "rpcz/server.hpp"
#include "rpcz/sync_event.hpp"

#include "proto/search.pb.h"
#include "proto/search.rpcz.h"

using namespace std;

namespace rpcz {

void super_done(SearchResponse *response,
               rpc_controller* newrpc,
               replier replier_copy) {
  delete newrpc;
  replier_copy.send(*response);
  delete response;
}

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

/*
Test all kinds of request interfaces.
. Sync or async
. Explicit deadline or implicit default deadline
. Return response or use output parameter (only for sync)
. Explicit error handler or implicit default error handler (only of async)
There are 4 sync interfaces and 4 async interfaces.
*/

TEST_F(server_test, SetDefaulDeadlineMs) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  SearchResponse response;
  request.set_query("timeout");
  stub.set_default_deadline_ms(1);
  try {
    stub.Search(request, &response);
    ASSERT_TRUE(false);
  } catch (rpc_error &error) {
    ASSERT_EQ(status::DEADLINE_EXCEEDED, error.get_status());
    return;
  }
  ASSERT_TRUE(false);
}

TEST_F(server_test, SyncRequest) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("stupid");
  SearchResponse response = stub.Search(request);
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for stupid", response.results(0));
}

TEST_F(server_test, SimpleRequestAsync) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  SearchResponse response;
  rpc_controller rpc_controller;
  request.set_query("happiness");
  sync_event sync;
  stub.Search(request, &response, &rpc_controller, new_callback(
          &sync, &sync_event::signal));
  sync.wait();
  ASSERT_TRUE(rpc_controller.ok());
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for happiness", response.results(0));
}

TEST_F(server_test, SimpleRequestWithError) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  request.set_query("foo");
  SearchResponse response;
  rpc_controller rpc_controller;
  stub.Search(request, &response, &rpc_controller, NULL);
  rpc_controller.wait();
  ASSERT_EQ(rpc_response_header::APPLICATION_ERROR, rpc_controller.get_status());
  ASSERT_EQ("I don't like foo.", rpc_controller.get_error_message());
}

TEST_F(server_test, SimpleRequestWithTimeout) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  SearchResponse response;
  rpc_controller rpc_controller;
  request.set_query("timeout");
  rpc_controller.set_deadline_ms(1);
  stub.Search(request, &response, &rpc_controller, NULL);
  rpc_controller.wait();
  ASSERT_EQ(rpc_response_header::DEADLINE_EXCEEDED, rpc_controller.get_status());
}

TEST_F(server_test, SimpleRequestWithTimeoutAsync) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  SearchResponse response;
  {
    rpc_controller rpc_controller;
    request.set_query("timeout");
    rpc_controller.set_deadline_ms(1);
    sync_event event;
    stub.Search(request, &response, &rpc_controller,
                new_callback(&event, &sync_event::signal));
    event.wait();
    ASSERT_EQ(rpc_response_header::DEADLINE_EXCEEDED, rpc_controller.get_status());
  }
}

TEST_F(server_test, EasyBlockingRequestRaisesExceptions) {
  SearchService_Stub stub(rpc_channel::create(*connection_), true);
  SearchRequest request;
  SearchResponse response;
  request.set_query("foo");
  try {
    stub.Search(request, &response);
    ASSERT_TRUE(false);
  } catch (rpc_error &error) {
    ASSERT_EQ(status::APPLICATION_ERROR, error.get_status());
    ASSERT_EQ(-4, error.get_application_error_code());
  }
}

}  // namespace
