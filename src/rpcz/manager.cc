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

#include <rpcz/manager.hpp>

#include <string>

#include <boost/lexical_cast.hpp>

#include <zmq.hpp>
#include <google/protobuf/stubs/common.h>

#include <rpcz/application_options.hpp>
#include <rpcz/broker_thread.hpp>
#include <rpcz/internal_commands.hpp>  // for kBind
#include <rpcz/logging.hpp>
#include <rpcz/router_service_factories.hpp>
#include <rpcz/sync_event.hpp>
#include <rpcz/worker/worker_thread_group.hpp>
#include <rpcz/zmq_utils.hpp>  // for send_empty_message

namespace rpcz {

manager::weak_ptr manager::this_weak_ptr_;
boost::mutex manager::this_weak_ptr_mutex_;

manager::manager()
  : context_(new zmq::context_t(1)),  // scope_ptr
    terminated_(new sync_event),  // scoped_ptr
    factories_(new router_service_factories),  // scoped_ptr
    next_event_id_(1) {
  DLOG(INFO) << "manager() ";
  frontend_endpoint_ = "inproc://" + boost::lexical_cast<std::string>(this)
      + ".rpcz.manager.frontend";

  // broker frontend socket
  zmq::socket_t* broker_fe_socket = new zmq::socket_t(*context_, ZMQ_ROUTER);
  int send_hwm = 10 * 1000 * 1000;
  broker_fe_socket->setsockopt(ZMQ_SNDHWM, &send_hwm, sizeof(send_hwm));
  int linger_ms = 0;
  broker_fe_socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  broker_fe_socket->bind(frontend_endpoint_.c_str());
  int threads = application_options::get_worker_threads();
  BOOST_ASSERT(threads > 0);
  worker_thread_group_.reset(new worker_thread_group(threads,
      frontend_endpoint_, *context_));
  sync_event event;
  broker_thread_ = boost::thread(&broker_thread::run,
                                 boost::ref(*context_), threads, &event,
                                 broker_fe_socket);
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
  BOOST_ASSERT(NULL == tss_fe_socket_.get());
  zmq::socket_t* socket = new zmq::socket_t(*context_, ZMQ_DEALER);
  int linger_ms = 0;
  socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  socket->connect(frontend_endpoint_.c_str());
  BOOST_ASSERT(NULL == tss_fe_socket_.get());
  tss_fe_socket_.reset(socket);  // set thread specific frontend socket
  return *socket;
}

void manager::bind(const std::string& endpoint,
    const service_factory_map_ptr& factories) {
  BOOST_ASSERT(factories);
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kBind, ZMQ_SNDMORE);
  send_string(&socket, endpoint, 0);
  message_iterator iter(socket);
  CHECK_EQ(0, iter.next().size());
  BOOST_ASSERT(iter.has_more());
  uint64 router_index(interpret_message<uint64>(iter.next()));
  BOOST_ASSERT(!iter.has_more());
  factories_->insert(router_index, factories);  // thread-safe
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
  terminated_->wait();  // wait until terminated
}

void manager::terminate() {
  terminated_->signal();
}

void manager::join_threads() {
  DLOG(INFO) << "join_threads()...";
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kQuit, 0);
  broker_thread_.join();
  worker_thread_group_->join_all();
  DLOG(INFO) << "All threads joined.";
}

manager::~manager() {
  DLOG(INFO) << "~manager()";
  join_threads();
}

}  // namespace rpcz
