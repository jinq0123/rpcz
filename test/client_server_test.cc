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

#include "proto/search.rpcz.h"

#include "rpcz/manager.hpp"
#include "rpcz/manager_ptr.hpp"
#include "rpcz/sync_event.hpp"
#include "rpcz/connection.hpp"

using namespace std;

namespace rpcz {

class delegate_resonder {
 public:
  explicit delegate_resonder(const replier& rep) :
      replier_(rep) {  // copy
  }
  void operator()(const rpc_error* e, const SearchResponse& resp) {
    if (e) return;
    replier_.reply(resp);
  }

 private:
  replier replier_;  // replier copy
};

class SearchServiceImpl : public SearchService {
 public:
  // Will take ownership of backend.
  SearchServiceImpl(connection_ptr backend)
      : backend_(backend), 
        manager_(manager::get()) {
  }

  ~SearchServiceImpl() {
  }

  virtual void Search(
      const SearchRequest& request,
      const replier& rep) {
    if (request.query() == "foo") {
      rep.reply_error(-4, "I don't like foo.");
    } else if (request.query() == "bar") {
      rep.reply_error(17, "I don't like bar.");
    } else if (request.query() == "delegate") {
      backend_.async_Search(request, delegate_resonder(rep));
      return;
    } else if (request.query() == "timeout") {
      // We "lose" the request. We are going to reply only when we get a request
      // for the query "delayed".
      boost::unique_lock<boost::mutex> lock(mu_);
      old_replier_.reset(new replier(rep));
      timeout_request_received.signal();
      return;
    } else if (request.query() == "delayed") {
      boost::unique_lock<boost::mutex> lock(mu_);
      if (old_replier_.get())
        old_replier_->reply(SearchResponse());
      rep.reply(SearchResponse());
    } else if (request.query() == "terminate") {
      rep.reply(SearchResponse());
      manager_->terminate();
    } else {
      SearchResponse response;
      response.add_results("The search for " + request.query());
      response.add_results("is great");
      rep.reply(response);
    }
  }

  rpcz::sync_event timeout_request_received;

 private:
  SearchService_Stub backend_;
  boost::mutex mu_;
  scoped_ptr<replier> old_replier_;
  manager_ptr manager_;
};

// For handling complex delegated queries.
class BackendSearchServiceImpl : public SearchService {
 public:
  virtual void Search(
      const SearchRequest&,
      const replier& rep) {
    SearchResponse response;
    response.add_results("42!");
    rep.reply(response);
  }
};

class server_test : public ::testing::Test {
 public:
  server_test() :
      context_(new zmq::context_t(1)) /* scoped_ptr */ {
    EXPECT_TRUE(manager::is_destroyed());
    application::set_zmq_context(context_.get());
    application::set_manager_threads(10);
    frontend_server_.reset(new server);
    backend_server_.reset(new server);
    start_server();
  }

  ~server_test() {
    // terminate the context, which will cause the thread to quit.
    application::terminate();
    frontend_server_.reset();
    backend_server_.reset();
    frontend_service_.reset();
    backend_service_.reset();
    frontend_connection_.reset();
    backend_connection_.reset();

    EXPECT_TRUE(manager::is_destroyed());
    context_.reset();
  }

  void start_server() {
    backend_service_.reset(new BackendSearchServiceImpl);
    backend_server_->register_singleton_service(*backend_service_);
    backend_server_->bind("inproc://myserver.backend");
    backend_connection_.reset(new connection(
        "inproc://myserver.backend"));

    frontend_service_.reset(new SearchServiceImpl(backend_connection_));
    frontend_server_->register_singleton_service(*frontend_service_);
    frontend_server_->bind("inproc://myserver.frontend");
    frontend_connection_.reset(new connection(
        "inproc://myserver.frontend"));
  }

protected:
  // destruct in reversed order
  scoped_ptr<zmq::context_t> context_;  // destruct last
  connection_ptr frontend_connection_;
  connection_ptr backend_connection_;
  scoped_ptr<SearchServiceImpl> frontend_service_;
  scoped_ptr<BackendSearchServiceImpl> backend_service_;
  // Server must destruct before service. (Or unregister services before destruct.)
  scoped_ptr<server> frontend_server_;
  scoped_ptr<server> backend_server_;
};

TEST_F(server_test, SimpleRequest) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  request.set_query("happiness");
  SearchResponse response = stub.Search(request);
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for happiness", response.results(0));
}

TEST_F(server_test, SimpleRequestAsync) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  request.set_query("happiness");

  struct hander {
    rpcz::sync_event sync;
    void operator()(const rpc_error* err, const SearchResponse& resp) {
      ASSERT_EQ(NULL, err);
      ASSERT_EQ(2, resp.results_size());
      ASSERT_EQ("The search for happiness", resp.results(0));
      sync.signal();
    }
  } hdl;
  stub.async_Search(request, boost::ref(hdl));
  hdl.sync.wait();
}

TEST_F(server_test, SimpleRequestWithError) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  request.set_query("foo");
  try {
    (void)stub.Search(request);
    ASSERT_TRUE(false);
  } catch (const rpc_error& e) {
    ASSERT_EQ("I don't like foo.", e.get_error_str());
  }
}

TEST_F(server_test, SimpleRequestWithTimeout) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  request.set_query("timeout");
  try {
    (void)stub.Search(request, 1);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(error_code::TIMEOUT_EXPIRED, error.get_error_code());
    return;
  }
  ASSERT_TRUE(false);
}

TEST_F(server_test, SimpleRequestWithTimeoutAsync) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  request.set_query("timeout");

  struct handler {
    rpcz::sync_event sync;

    void operator()(const rpc_error* error, const SearchResponse &) {
      ASSERT_TRUE(NULL != error);
      ASSERT_EQ(error_code::TIMEOUT_EXPIRED, error->get_error_code());
      sync.signal();
    }
  } hdl;

  stub.async_Search(request,
      boost::ref(hdl),
      1/*timeout ms*/);
  hdl.sync.wait();
}

TEST_F(server_test, DelegatedRequest) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  request.set_query("delegate");
  SearchResponse response = stub.Search(request);
  ASSERT_EQ("42!", response.results(0));
}

TEST_F(server_test, EasyBlockingRequestRaisesExceptions) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  SearchResponse response;
  request.set_query("foo");
  try {
    stub.Search(request, &response);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(-4, error.get_error_code());
  }
}

TEST_F(server_test, EasyBlockingRequestWithTimeout) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  SearchResponse response;
  request.set_query("timeout");
  try {
    stub.Search(request, 1, &response);
    ASSERT_TRUE(false);
  } catch (const rpc_error& error) {
    ASSERT_EQ(error_code::TIMEOUT_EXPIRED, error.get_error_code());
  }
  // We may get here before the timing out request was processed, and if we
  // just send delay right away, the server may be unable to reply.
  frontend_service_->timeout_request_received.wait();
  request.set_query("delayed");
  stub.Search(request, &response);
}

TEST_F(server_test, ConnectionManagerTermination) {
  SearchService_Stub stub(frontend_connection_);
  SearchRequest request;
  request.set_query("terminate");
  try {
    stub.Search(request, 1/*ms*/);
  } catch (const rpc_error& error) {
    ASSERT_EQ(error_code::TIMEOUT_EXPIRED, error.get_error_code());
  }
  LOG(INFO)<<"I'm here";
  manager::get()->run();
  LOG(INFO)<<"I'm there";
}

}  // namespace
