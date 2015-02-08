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

#include "proto/search.rpcz.h"

#include "rpcz/manager.hpp"
#include "rpcz/manager_ptr.hpp"
#include "rpcz/sync_event.hpp"
#include "rpcz/connection.hpp"

using namespace std;

namespace rpcz {

class SearchServiceImpl : public SearchService {
 public:
  SearchServiceImpl() {
    LOG(INFO) << "SearchServiceImpl()";
  }

  ~SearchServiceImpl() {
    LOG(INFO) << "~SearchServiceImpl()";
  }

  virtual void Search(
      const SearchRequest& request,
      const responder& rspndr) {
    LOG(INFO) << "Search()";

    if (request.query() == "timeout") {
      // We "lose" the request.
      return;
    }

    SearchResponse response;
    response.add_results("The search for " + request.query());
    response.add_results("is great");
    rspndr.respond(response);
  }
};

class server_test : public ::testing::Test {
 public:
  server_test() :
      context_(new zmq::context_t(1)) /* scoped_ptr */ {
    LOG(INFO) << "server_test()";
    EXPECT_TRUE(manager::is_destroyed());
    application::set_zmq_context(context_.get());
    application::set_manager_threads(10);
    server_.reset(new server);
    start_server();
  }

  ~server_test() {
    LOG(INFO) << "~server_test()";
    // terminate the context, which will cause the thread to quit.
    application::terminate();
    server_.reset();
    service_.reset();
    connection_.reset();

    EXPECT_TRUE(manager::is_destroyed());
    context_.reset();
  }

  void start_server() {
    LOG(INFO) << "start_server()";
    service_.reset(new SearchServiceImpl);
    server_->register_singleton_service(*service_);
    server_->bind("inproc://myserver.frontend");
    connection_.reset(new connection(
        "inproc://myserver.frontend"));
  }

protected:
  // destruct in reversed order
  scoped_ptr<zmq::context_t> context_;  // destruct last
  connection_ptr connection_;
  scoped_ptr<SearchServiceImpl> service_;
  // Server must destruct before service. (Or unregister services before destruct.)
  scoped_ptr<server> server_;
};

struct handler {
  rpcz::sync_event sync;
  SearchResponse response;
  boost::optional<rpc_error> error;

  void operator()(const rpc_error* e, const SearchResponse& resp)
  {
    LOG(INFO) << "handler operator()()";
    if (e) {
      error.reset(*e);
    } else {
      response = resp;
    }
    sync.signal();
  }
};

/*
Test all kinds of request interfaces.
. Sync or async
. Explicit timeout or implicit default timeout
. Return response or use output parameter (only for sync)
. Explicit error handler or implicit default error handler (only for async)
There are 4 sync interfaces and 4 async interfaces.

Ordered as the declaration in search.rpcz.h.
*/

// Async interfaces:

TEST_F(server_test, AsyncRequestWithTimeout) {
  LOG(INFO) << "AsyncRequestWithTimeout";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("timeout");
  handler hdl;
  stub.async_Search(request,
      boost::ref(hdl),
      1/*ms*/);
  hdl.sync.wait();
  ASSERT_TRUE(hdl.error);
  ASSERT_EQ(error_code::TIMEOUT_EXPIRED, hdl.error->get_error_code());
}

TEST_F(server_test, AsyncRequest) {
  LOG(INFO) << "AsyncRequest";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("stone");
  handler hdl;
  stub.async_Search(request, boost::ref(hdl));
  hdl.sync.wait();

  ASSERT_FALSE(hdl.error);
  ASSERT_EQ(2, hdl.response.results_size());
  ASSERT_EQ("The search for stone", hdl.response.results(0));
}

TEST_F(server_test, AsyncOnewayRequest) {
  LOG(INFO) << "AsyncOnewayRequest";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("rocket");
  stub.async_Search(request, 0/*ms*/);
  stub.async_Search(request, 1/*ms*/);
  stub.async_Search(request, 10/*ms*/);
  stub.async_Search(request, 1000/*ms*/);
  stub.async_Search(request, 10000/*ms*/);
  stub.async_Search(request, -1/*ms*/);

  // Sync request to end. Crash on reset if not.
  (void)stub.Search(request);(void)stub.Search(request);(void)stub.Search(request);
  (void)stub.Search(request);(void)stub.Search(request);(void)stub.Search(request);
}

TEST_F(server_test, AsyncOnewayRequestDefaultMs) {
  LOG(INFO) << "AsyncOnewayRequestDefaultMs";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("robot");
  stub.async_Search(request);

  // Sync request to end. Crash on reset if not.
  (void)stub.Search(request);(void)stub.Search(request);(void)stub.Search(request);
  (void)stub.Search(request);(void)stub.Search(request);(void)stub.Search(request);
}

// Sync interfaces:

TEST_F(server_test, SyncRequest) {
  LOG(INFO) << "SyncRequest";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("student");
  SearchResponse response;
  stub.Search(request, 5000/*ms*/, &response);
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for student", response.results(0));
}

TEST_F(server_test, SyncRequestDefaultMs) {
  LOG(INFO) << "SyncRequestDefaultMs";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("stupid");
  SearchResponse response;
  stub.Search(request, &response);
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for stupid", response.results(0));
}

TEST_F(server_test, SyncRequestReturn) {
  LOG(INFO) << "SyncRequestReturn";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("spool");
  SearchResponse response = stub.Search(request, 5000/*ms*/);
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for spool", response.results(0));
}

TEST_F(server_test, SyncRequestReturnDefaultMs) {
  LOG(INFO) << "SyncRequestReturnDefaultMs";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  request.set_query("star");
  SearchResponse response = stub.Search(request);
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for star", response.results(0));
}

// Other interfaces:

TEST_F(server_test, SetDefaulTimeoutMs) {
  LOG(INFO) << "SetDefaulDeadlineMs";
  SearchService_Stub stub(connection_);
  SearchRequest request;
  SearchResponse response;
  request.set_query("timeout");
  stub.set_default_timeout_ms(1);
  try {
    stub.Search(request, &response);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(error_code::TIMEOUT_EXPIRED, error.get_error_code());
    return;
  }
  ASSERT_TRUE(false);
}

}  // namespace
