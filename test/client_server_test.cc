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
#include "rpcz/rpc_channel.hpp"
#include "rpcz/rpc_controller.hpp"
#include "rpcz/server.hpp"
#include "rpcz/sync_event.hpp"

#include "proto/search.pb.h"
#include "proto/search.rpcz.h"

using namespace std;

namespace rpcz {

void super_done(SearchResponse *response,
               rpc_controller* newrpc, reply<SearchResponse> reply) {
  delete newrpc;
  reply.send(*response);
  delete response;
}

class SearchServiceImpl : public SearchService {
 public:
  // Will take ownership of backend.
  SearchServiceImpl(SearchService_Stub* backend)
      : backend_(backend), delayed_reply_(NULL)
      , cm_(connection_manager::get()) {};

  ~SearchServiceImpl() {
  }

  virtual void Search(
      const SearchRequest& request,
      reply<SearchResponse> reply) {
    if (request.query() == "foo") {
      reply.Error(-4, "I don't like foo.");
    } else if (request.query() == "bar") {
      reply.Error(17, "I don't like bar.");
    } else if (request.query() == "delegate") {
      rpc_controller* newrpc = new rpc_controller;
      SearchResponse* response = new SearchResponse;
      backend_->Search(request, response, newrpc, new_callback(super_done,
                                                               response,
                                                               newrpc,
                                                               reply));
      return;
    } else if (request.query() == "timeout") {
      // We "lose" the request. We are going to reply only when we get a request
      // for the query "delayed".
      boost::unique_lock<boost::mutex> lock(mu_);
      delayed_reply_ = reply;
      timeout_request_received.signal();
      return;
    } else if (request.query() == "delayed") {
      boost::unique_lock<boost::mutex> lock(mu_);
      delayed_reply_.send(SearchResponse());
      reply.send(SearchResponse());
    } else if (request.query() == "terminate") {
      reply.send(SearchResponse());
      cm_->terminate();
    } else {
      SearchResponse response;
      response.add_results("The search for " + request.query());
      response.add_results("is great");
      reply.send(response);
    }
  }

  sync_event timeout_request_received;

 private:
  scoped_ptr<SearchService_Stub> backend_;
  boost::mutex mu_;
  reply<SearchResponse> delayed_reply_;
  connection_manager_ptr cm_;
};

// For handling complex delegated queries.
class BackendSearchServiceImpl : public SearchService {
 public:
  virtual void Search(
      const SearchRequest&,
      reply<SearchResponse> reply) {
    SearchResponse response;
    response.add_results("42!");
    reply.send(response);
  }
};

class server_test : public ::testing::Test {
 public:
  server_test() :
      context_(new zmq::context_t(1)) /* scoped_ptr */ {
    EXPECT_EQ(0, connection_manager::use_count());
    application::set_zmq_context(context_.get());
    application::set_connection_manager_threads(10);
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
    EXPECT_EQ(0, connection_manager::use_count());
    context_.reset();
  }

  void start_server() {
    backend_service_.reset(new BackendSearchServiceImpl);
    backend_server_->register_service(backend_service_.get());
    backend_server_->bind("inproc://myserver.backend");
    rpcz::connection_manager_ptr cm = rpcz::connection_manager::get();
    backend_connection_ = cm->connect("inproc://myserver.backend");

    frontend_service_.reset(new SearchServiceImpl(
        new SearchService_Stub(rpc_channel::create(backend_connection_), true)));
    frontend_server_->register_service(frontend_service_.get());
    frontend_server_->bind("inproc://myserver.frontend");
    frontend_connection_ = cm->connect("inproc://myserver.frontend");
  }

  SearchResponse send_blocking_request(connection connection,
                                       const std::string& query) {
    SearchService_Stub stub(rpc_channel::create(connection), true);
    SearchRequest request;
    SearchResponse response;
    request.set_query(query);
    rpc_controller rpc_controller;
    stub.Search(request, &response, &rpc_controller, NULL);
    rpc_controller.wait();
    EXPECT_TRUE(rpc_controller.ok());
    return response;
  }

 protected:
  scoped_ptr<zmq::context_t> context_;
  connection frontend_connection_;
  connection backend_connection_;
  scoped_ptr<server> frontend_server_;
  scoped_ptr<server> backend_server_;
  scoped_ptr<SearchServiceImpl> frontend_service_;
  scoped_ptr<BackendSearchServiceImpl> backend_service_;
};

TEST_F(server_test, SimpleRequest) {
  SearchResponse response =
      send_blocking_request(frontend_connection_, "happiness");
  ASSERT_EQ(2, response.results_size());
  ASSERT_EQ("The search for happiness", response.results(0));
}

TEST_F(server_test, SimpleRequestAsync) {
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
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
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
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
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
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
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
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

TEST_F(server_test, DelegatedRequest) {
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
  SearchRequest request;
  SearchResponse response;
  rpc_controller rpc_controller;
  request.set_query("delegate");
  stub.Search(request, &response, &rpc_controller, NULL);
  rpc_controller.wait();
  ASSERT_EQ(rpc_response_header::OK, rpc_controller.get_status());
  ASSERT_EQ("42!", response.results(0));
}

TEST_F(server_test, EasyBlockingRequestUsingDelegate) {
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
  SearchRequest request;
  SearchResponse response;
  request.set_query("delegate");
  stub.Search(request, &response);
  ASSERT_EQ("42!", response.results(0));
}

TEST_F(server_test, EasyBlockingRequestRaisesExceptions) {
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
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

TEST_F(server_test, EasyBlockingRequestWithTimeout) {
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
  SearchRequest request;
  SearchResponse response;
  request.set_query("timeout");
  try {
    stub.Search(request, &response, 1);
    ASSERT_TRUE(false);
  } catch (rpc_error &error) {
    ASSERT_EQ(status::DEADLINE_EXCEEDED, error.get_status());
  }
  // We may get here before the timing out request was processed, and if we
  // just send delay right away, the server may be unable to reply.
  frontend_service_->timeout_request_received.wait();
  request.set_query("delayed");
  stub.Search(request, &response);
}

TEST_F(server_test, ConnectionManagerTermination) {
  SearchService_Stub stub(rpc_channel::create(frontend_connection_), true);
  SearchRequest request;
  request.set_query("terminate");
  SearchResponse response;
  try {
    stub.Search(request, &response, 1);
  } catch (rpc_error &error) {
    ASSERT_EQ(status::DEADLINE_EXCEEDED, error.get_status());
  }
  LOG(INFO)<<"I'm here";
  connection_manager::get()->run();
  LOG(INFO)<<"I'm there";
}
}  // namespace
