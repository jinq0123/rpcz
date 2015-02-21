// Copyright 2011 Google Inc. All Rights Reserved.
// Copyright 2015 Jin Qing.
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

#include <stdio.h>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <glog/logging.h>
#include <zmq.hpp>

#include "gtest/gtest.h"
#include "rpcz/application.hpp"
#include "rpcz/application_error_code.hpp"
#include "rpcz/callback.hpp"
#include "rpcz/connection.hpp"
#include "rpcz/connection_ptr.hpp"
#include "rpcz/manager.hpp"
#include "rpcz/rpc_controller.hpp"
#include "rpcz/rpc_error.hpp"
#include "rpcz/sync_event.hpp"
#include "rpcz/zmq_utils.hpp"
#include "rpcz/service_factory_map.hpp"

namespace rpcz {

class manager_test : public ::testing::Test {
 public:
  manager_test() {
  }
  ~manager_test() {
  }

 protected:
};

// friend function of connection.
void request_connection(connection& conn,
    message_vector& data, rpc_controller* ctrl)
{
    conn.request(data, ctrl);
}

TEST_F(manager_test, TestStartsAndFinishes) {
  ASSERT_TRUE(manager::is_destroyed());
  application::set_worker_threads(4);
  manager_ptr mgr = manager::get();
}

// Sink server will not reply.
void sink_server() {
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_DEALER);
  // Use tcp because inproc must under the same zmq context.
  socket.bind("tcp://*:5555");

  bool should_quit = false;
  while (!should_quit) {
    message_vector v;
    GOOGLE_CHECK(read_message_to_vector(&socket, &v));
    ASSERT_EQ(3, v.size());
    if (message_to_string(v[1]) == "hello") {
      ASSERT_EQ("there", message_to_string(v[2]).substr(0, 5));
    } else if (message_to_string(v[1]) == "QUIT") {
      should_quit = true;
    } else {
      GOOGLE_CHECK(false) << "Unknown command: " << message_to_string(v[1]);
    }
  }
}

message_vector* create_simple_request(int number=0) {
  message_vector* request = new message_vector;
  request->push_back(string_to_message("hello"));
  char str[256];
  sprintf(str, "there_%d", number);
  request->push_back(string_to_message(str));
  return request;
}

message_vector* create_quit_request() {
  message_vector* request = new message_vector;
  request->push_back(string_to_message("QUIT"));
  request->push_back(string_to_message(""));
  return request;
}

TEST_F(manager_test, TestTimeoutAsync) {
  ASSERT_TRUE(manager::is_destroyed());
  application::set_worker_threads(4);
  manager_ptr mgr = manager::get();
  zmq::context_t context(1);
  zmq::socket_t server(context, ZMQ_DEALER);
  server.bind("tcp://*:5555");
  connection_ptr conn(new connection("tcp://localhost:5555"));
  scoped_ptr<message_vector> request(create_simple_request());

  struct handler {
    rpcz::sync_event event;
    void operator()(const rpc_error* error, const void*, size_t) {
      ASSERT_TRUE(NULL != error);
      ASSERT_EQ(error_code::TIMEOUT_EXPIRED, error->get_error_code());
      event.signal();
    }
  } hdl;

  request_connection(*conn, *request,
      new rpc_controller(0, boost::ref(hdl), 0));
  hdl.event.wait();
}

class barrier_handler {
 public:
  barrier_handler() : count_(0) {}

  void operator()(const rpc_error*, const void*, size_t) {
    boost::unique_lock<boost::mutex> lock(mutex_);
    ++count_;
    cond_.notify_all();
  }

  virtual void wait(int n) {
    boost::unique_lock<boost::mutex> lock(mutex_);
    while (count_ < n) {
      cond_.wait(lock);
    }
  }

 private:
  boost::mutex mutex_;
  boost::condition_variable cond_;
  int count_;
};

void SendManyMessages(const connection_ptr& conn) {
  const int request_count = 1500;
  barrier_handler barrier;
  for (int i = 0; i < request_count; ++i) {
    message_vector* request = create_simple_request(i);
    request_connection(*conn, *request,
        new rpc_controller(i, boost::ref(barrier), 0/*ms*/));
  }
  barrier.wait(request_count);
}

TEST_F(manager_test, ManyClientsTest) {
  ASSERT_TRUE(manager::is_destroyed());
  application::set_worker_threads(4);
  manager_ptr mgr = manager::get();

  boost::thread thrd(sink_server);

  connection_ptr conn(new connection("tcp://localhost:5555"));
  SendManyMessages(conn);
  scoped_ptr<message_vector> request(create_quit_request());
  struct handler {
    rpcz::sync_event event;
    void operator()(const rpc_error*, const void*, size_t) {
      event.signal();
    }
  } hdl;
  request_connection(*conn, *request,
      new rpc_controller(12345678, boost::ref(hdl), 0/*ms*/));
  hdl.event.wait();
  thrd.join();
}

TEST_F(manager_test, TestUnbind) {
  ASSERT_TRUE(manager::is_destroyed());
  manager_ptr mgr = manager::get();
  const char kEndpoint[] = "inproc://server.point";
  mgr->bind(kEndpoint, boost::make_shared<service_factory_map>());
  mgr->unbind(kEndpoint);
}

const static char* kEndpoint = "inproc://test";
const static char* kReply = "gotit";

void DoThis(zmq::context_t* context) {
  assert(context);
  LOG(INFO)<<"Creating socket. Context="<<context;
  zmq::socket_t socket(*context, ZMQ_PUSH);
  socket.connect(kEndpoint);
  send_string(&socket, kReply);
  socket.close();
  LOG(INFO)<<"socket closed";
}

TEST_F(manager_test, ProcessesSingleCallback) {
  ASSERT_TRUE(manager::is_destroyed());
  application::set_worker_threads(4);
  manager_ptr mgr = manager::get();
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_PULL);
  socket.bind(kEndpoint);
  mgr->add(new_callback(&DoThis, &context));
  message_vector messages;
  CHECK(read_message_to_vector(&socket, &messages));
  ASSERT_EQ(1, messages.size());
  CHECK_EQ(kReply, message_to_string(messages[0]));
}

void Increment(boost::mutex* mu,
               boost::condition_variable* cond, int* x) {
  mu->lock();
  (*x)++;
  cond->notify_one();
  mu->unlock();
}

void add_many_closures() {
  boost::mutex mu;
  boost::condition_variable cond;
  boost::unique_lock<boost::mutex> lock(mu);
  int x = 0;
  const int kMany = 137;
  manager_ptr mgr = manager::get();
  for (int i = 0; i < kMany; ++i) {
    mgr->add(new_callback(&Increment, &mu, &cond, &x));
  }
  CHECK_EQ(0, x);  // since we are holding the lock
  while (x != kMany) {
    cond.wait(lock);
  }
}

TEST_F(manager_test, ProcessesManyCallbacksFromManyThreads) {
  ASSERT_TRUE(manager::is_destroyed());
  const int thread_count = 10;
  application::set_worker_threads(thread_count);
  manager_ptr mgr = manager::get();
  boost::thread_group thread_group;
  for (int i = 0; i < thread_count; ++i) {
    thread_group.add_thread(
        new boost::thread(add_many_closures));
  }
  thread_group.join_all();
}
}  // namespace rpcz
