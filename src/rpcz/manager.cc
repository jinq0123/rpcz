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

#include <rpcz/manager.hpp>

#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include <zmq.hpp>
#include <google/protobuf/stubs/common.h>

#include <rpcz/application_options.hpp>
#include <rpcz/broker_thread.hpp>
#include <rpcz/internal_commands.hpp>  // for kConnect
#include <rpcz/logging.hpp>
#include <rpcz/sync_event.hpp>
#include <rpcz/worker_thread_fun.hpp>  // for worker_thread_fun
#include <rpcz/zmq_utils.hpp>  // for send_empty_message

namespace rpcz {

manager::weak_ptr manager::this_weak_ptr_;
boost::mutex manager::this_weak_ptr_mutex_;

manager::manager()
  : context_(NULL),
    is_terminating_(new sync_event) { // scoped_ptr
  DLOG(INFO) << "manager() ";
  frontend_endpoint_ = "inproc://" + boost::lexical_cast<std::string>(this)
      + ".rpcz.manager.frontend";

  application_options options;
  context_ = options.get_zmq_context();
  if (NULL == context_) {
    int zmq_io_threads = options.get_zmq_io_threads();
    assert(zmq_io_threads > 0);
    own_context_.reset(new zmq::context_t(zmq_io_threads));
    context_ = own_context_.get();
  }
  assert(context_);
  zmq::socket_t* frontend_socket = new zmq::socket_t(*context_, ZMQ_ROUTER);
  int linger_ms = 0;
  frontend_socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  frontend_socket->bind(frontend_endpoint_.c_str());
  int nthreads = options.get_worker_threads();
  assert(nthreads > 0);
  // XXX delete workers and rename broker_thread to worker_thread...master
  for (int i = 0; i < nthreads; ++i) {
    worker_threads_.add_thread(new boost::thread(worker_thread_fun,
        boost::ref(*context_), frontend_endpoint_));
  }
  sync_event event;
  broker_thread_ = boost::thread(&broker_thread::run,
                                 boost::ref(*context_), nthreads, &event,
                                 frontend_socket);
  event.wait();
}

manager_ptr manager::get_new() {
  lock_guard lock(this_weak_ptr_mutex_);
  manager_ptr p = this_weak_ptr_.lock();
  if (p) return p;
  p.reset(new manager);
  this_weak_ptr_ = p;
  return p;
}

bool manager::is_destroyed() {
  return 0 == this_weak_ptr_.use_count();
}

// used by get_frontend_socket()
zmq::socket_t& manager::new_frontend_socket() {
  assert(NULL == socket_.get());
  zmq::socket_t* socket = new zmq::socket_t(*context_, ZMQ_DEALER);
  int linger_ms = 0;
  socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  socket->connect(frontend_endpoint_.c_str());
  assert(NULL == socket_.get());
  socket_.reset(socket);  // set thread specific socket
  return *socket;
}

void manager::bind(const std::string& endpoint,
    const service_factory_map& factories) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kBind, ZMQ_SNDMORE);
  send_string(&socket, endpoint, ZMQ_SNDMORE);
  send_pointer(&socket, &factories, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
}

// Unbind socket and unregister server_function.
void manager::unbind(const std::string& endpoint) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kUnbind, ZMQ_SNDMORE);
  send_string(&socket, endpoint, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
}

void manager::add(closure* closure) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kRunClosure, ZMQ_SNDMORE);
  send_pointer(&socket, closure, 0);
  return;
}

void manager::run() {
  is_terminating_->wait();
}

void manager::terminate() {
  is_terminating_->signal();
}

manager::~manager() {
  DLOG(INFO) << "~manager()";
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kQuit, 0);
  broker_thread_.join();
  worker_threads_.join_all();
  DLOG(INFO) << "All threads joined.";
}

}  // namespace rpcz
