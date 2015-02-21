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

#include <rpcz/broker_thread.hpp>

#include <rpcz/callback.hpp>
#include <rpcz/clock.hpp>  // for zclock_ms()
#include <rpcz/connection_info_hash.hpp>  // for hash_value()
#include <rpcz/connection_info_zmq.hpp>  // for read_connection_info()
#include <rpcz/internal_commands.hpp>
#include <rpcz/logging.hpp>
#include <rpcz/rpc_controller.hpp>  // for rpc_controller
#include <rpcz/sync_event.hpp>
#include <rpcz/zmq_utils.hpp>

namespace rpcz {

broker_thread::broker_thread(
    zmq::context_t& context,
    int nthreads,
    sync_event* ready_event,
    zmq::socket_t* frontend_socket)
    : context_(context),
    frontend_socket_(frontend_socket) {
  BOOST_ASSERT(nthreads > 0);
  // Index 0 is reserved for debug check.
  dealer_sockets_.push_back(NULL);
  BOOST_ASSERT(1 == dealer_sockets_.size());
  router_sockets_.push_back(NULL);
  BOOST_ASSERT(1 == router_sockets_.size());

  wait_for_workers_ready_reply(nthreads);
  ready_event->signal();
  reactor_.add_socket(frontend_socket, new_permanent_callback(
      this, &broker_thread::handle_frontend_socket,
      frontend_socket));
}

void broker_thread::wait_for_workers_ready_reply(int nthreads) {
  BOOST_ASSERT(nthreads > 0);
  BOOST_ASSERT(workers_.empty());
  workers_.resize(nthreads);
  for (int i = 0; i < nthreads; ++i) {
    message_iterator iter(*frontend_socket_);
    std::string sender = message_to_string(iter.next());
    BOOST_ASSERT(!sender.empty());  // zmq id
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    CHECK_EQ(c2b::kWorkerReady, command)
        << "Got unexpected command " << (int)command;
    BOOST_ASSERT(iter.has_more());
    uint64 worker_index(interpret_message<uint64>(iter.next()));
    BOOST_ASSERT(!iter.has_more());
    BOOST_ASSERT(worker_index < workers_.size());
    BOOST_ASSERT(workers_[worker_index].empty());
    workers_[worker_index] = sender;
  }
}

void broker_thread::run(zmq::context_t& context,
    int nthreads, sync_event* ready_event,
    zmq::socket_t* frontend_socket) {
  broker_thread bt(
      context, nthreads, ready_event, frontend_socket);
  bt.reactor_.loop();
}

void broker_thread::handle_frontend_socket(zmq::socket_t* frontend_socket) {
  message_iterator iter(*frontend_socket);
  std::string sender = message_to_string(iter.next());
  CHECK_EQ(0, iter.next().size());
  char command(interpret_message<char>(iter.next()));
  using namespace c2b;  // command to broker
  switch (command) {
    case kQuit:
      handle_quit_command(iter);
      break;
    case kConnect:
      handle_connect_command(sender, message_to_string(iter.next()));
      break;
    case kBind:
      handle_bind_command(sender, iter);
      break;
    case kUnbind:
      handle_unbind_command(sender, message_to_string(iter.next()));
      break;
    case kRequest:
      send_request(iter);
      break;
    case kReply:
      send_reply(iter);
      break;
    case kRunClosure:
      add_closure(interpret_message<closure*>(iter.next()));
      break;
    case kRegisterSvc:
      register_service(iter);
      break;

    // worker to broker commands
    case kWorkerReady:
      CHECK(false);
      break;
    case kWorkerDone:
      handle_worker_done_command(sender);
      break;

    default:
      BOOST_ASSERT(false);
      break;
  }  // switch
}

// command must be broker to worker (b2w) command.
inline void broker_thread::begin_worker_command(
    const connection_info& conn_info, char command) {
  begin_worker_command(get_worker_index(conn_info), command);
}

// command must be broker to worker (b2w) command.
inline void broker_thread::begin_worker_command(
    size_t worker_index, char command) {
  BOOST_ASSERT(worker_index < workers_.size());
  send_string(frontend_socket_, workers_[worker_index], ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_char(frontend_socket_, command, ZMQ_SNDMORE);
}

// Add closure to random worker thread.
inline void broker_thread::add_closure(closure* closure) {
  BOOST_ASSERT(!workers_.empty());
  size_t worker_index = rand() % workers_.size();
  begin_worker_command(worker_index, b2w::kRunClosure);
  send_pointer(frontend_socket_, closure, 0);
}

void broker_thread::handle_connect_command(
      const std::string& sender, const std::string& endpoint) {
  BOOST_ASSERT(!dealer_sockets_.empty());  // idx 0 is reserved
  zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_DEALER);
  dealer_sockets_.push_back(socket);
  uint64 dealer_index = dealer_sockets_.size() - 1;
  int linger_ms = 0;
  socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  socket->connect(endpoint.c_str());
  reactor_.add_socket(socket, new_permanent_callback(
          this, &broker_thread::handle_dealer_socket, dealer_index));

  send_string(frontend_socket_, sender, ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_uint64(frontend_socket_, dealer_index, 0);
}

void broker_thread::handle_bind_command(
    const std::string& sender, message_iterator& iter) {
  std::string endpoint(message_to_string(iter.next()));
  handle_bind_command(sender, endpoint);
}

void broker_thread::handle_bind_command(
    const std::string& sender,
    const std::string& endpoint) {
  zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_ROUTER);  // delete in reactor
  int linger_ms = 0;
  socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  socket->bind(endpoint.c_str());  // TODO: catch exception
  BOOST_ASSERT(!router_sockets_.empty());  // Index 0 is reserved.
  uint64 router_index = router_sockets_.size();
  router_sockets_.push_back(socket);
  bind_map_[endpoint] = socket;  // for unbind
  // reactor will own socket and callback.
  reactor_.add_socket(socket, new_permanent_callback(
      this, &broker_thread::handle_router_socket, router_index));

  send_string(frontend_socket_, sender, ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_uint64(frontend_socket_, router_index);
}

void broker_thread::handle_unbind_command(
    const std::string& sender,
    const std::string& endpoint) {
  endpoint_to_socket::const_iterator it = bind_map_.find(endpoint);
  if (it == bind_map_.end()) return;
  assert((*it).second);
  reactor_.del_socket((*it).second,
      new_callback(this, &broker_thread::handle_socket_deleted, std::string(sender)));
  bind_map_.erase(it);
  // Socket is not delelted yet.
  // It will callback on deleted before next zmq_poll().
}

void broker_thread::handle_quit_command(message_iterator& iter) {
  // Ask the workers to quit. They'll in turn send kWorkerDone.
  for (size_t i = 0; i < workers_.size(); ++i) {
    send_string(frontend_socket_, workers_[i], ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_char(frontend_socket_, b2w::kWorkerQuit, 0);
  }
}

void broker_thread::handle_worker_done_command(
    const std::string& sender) {
  workers_.erase(std::remove(workers_.begin(), workers_.end(), sender));
  if (!workers_.empty())
    return;
  // All workers are gone, time to quit.
  reactor_.set_should_quit();
}

void broker_thread::handle_socket_deleted(const std::string sender) {
  send_string(frontend_socket_, sender, ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, 0);  // Only to end zmq message?
}

void broker_thread::handle_router_socket(uint64 router_index) {
  BOOST_ASSERT(is_router_index_legal(router_index));
  message_iterator iter(*router_sockets_[router_index]);
  if (!iter.has_more()) return;
  std::string sender(message_to_string(iter.next()));
  if (!iter.has_more()) return;
  if (iter.next().size() != 0) return;
  if (!iter.has_more()) return;

  connection_info info = { true, router_index, sender };
  begin_worker_command(info, b2w::kHandleData);
  write_connection_info(frontend_socket_, info, ZMQ_SNDMORE);
  forward_messages(iter, *frontend_socket_);
}

void broker_thread::handle_dealer_socket(uint64 dealer_index) {
  BOOST_ASSERT(is_dealer_index_legal(dealer_index));
  message_iterator iter(*dealer_sockets_[dealer_index]);
  if (!iter.has_more()) return;
  if (iter.next().size() != 0) return;
  if (!iter.has_more()) return;

  connection_info info = { false, dealer_index, "" };
  begin_worker_command(info, b2w::kHandleData);
  write_connection_info(frontend_socket_, info, ZMQ_SNDMORE);
  forward_messages(iter, *frontend_socket_);
}

void broker_thread::handle_timeout(uint64 event_id, size_t worker_index) {
  BOOST_ASSERT(!workers_.empty());
  worker_index %= workers_.size();
  begin_worker_command(worker_index, b2w::kHandleTimeout);
  send_uint64(frontend_socket_, event_id);
}

inline void broker_thread::send_request(message_iterator& iter) {
  BOOST_ASSERT(iter.has_more());
  connection_info info;
  read_connection_info(iter, &info);
  BOOST_ASSERT(is_connection_info_legal(info));
  BOOST_ASSERT(iter.has_more());
  rpc_controller* ctrl = interpret_message<rpc_controller*>(iter.next());
  BOOST_ASSERT(ctrl);
  uint64 event_id = ctrl->get_event_id();

  int64 timeout_ms = ctrl->get_timeout_ms();
  if (-1 != timeout_ms) {
    // XXX when to delete timeout handler?
    reactor_.run_closure_at(zclock_ms() + timeout_ms,
        new_callback(this, &broker_thread::handle_timeout,
            event_id, get_worker_index(info)));
  }
  forward_to(info, iter);  // Request remote server.

  start_rpc(info, ctrl);  // Let worker thread start this rpc
}

inline void broker_thread::start_rpc(
    const connection_info& info,
    const rpc_controller* ctrl) {
  begin_worker_command(info, b2w::kStartRpc);
  send_pointer(frontend_socket_, ctrl);
}

inline void broker_thread::send_reply(message_iterator& iter) {
  BOOST_ASSERT(iter.has_more());
  connection_info info;
  read_connection_info(iter, &info);
  BOOST_ASSERT(is_connection_info_legal(info));
  forward_to(info, iter);
}

void broker_thread::register_service(message_iterator& iter) {
  BOOST_ASSERT(iter.has_more());
  connection_info info;
  read_connection_info(iter, &info);
  BOOST_ASSERT(is_connection_info_legal(info));

  // forward to worker thread
  begin_worker_command(info, b2w::kRegisterSvc);
  write_connection_info(frontend_socket_, info, ZMQ_SNDMORE);
  forward_messages(iter, *frontend_socket_);
}

bool broker_thread::is_dealer_index_legal(uint64 dealer_index) const {
  // Index 0 is reserved.
  return dealer_index > 0 && dealer_index < dealer_sockets_.size();
}

bool broker_thread::is_router_index_legal(uint64 router_index) const {
  // Index 0 is reserved.
  return router_index > 0 && router_index < router_sockets_.size();
}

bool broker_thread::is_connection_info_legal(const connection_info& info) const {
  if (info.is_router)
    return is_router_index_legal(info.index);
  return is_dealer_index_legal(info.index);
}

inline void broker_thread::forward_to(
    const connection_info& conn_info,
    message_iterator& iter) {
  BOOST_ASSERT(is_connection_info_legal(conn_info));
  if (conn_info.is_router) {
    zmq::socket_t* socket = router_sockets_[conn_info.index];
    BOOST_ASSERT(socket);
    send_string(socket, conn_info.sender, ZMQ_SNDMORE);
    send_empty_message(socket, ZMQ_SNDMORE);
    forward_messages(iter, *socket);
    return;
  }
  zmq::socket_t* socket = dealer_sockets_[conn_info.index];
  BOOST_ASSERT(socket);
  send_empty_message(socket, ZMQ_SNDMORE);
  forward_messages(iter, *socket);
}

// Map connection to worker thread.
inline size_t broker_thread::get_worker_index(
    const connection_info& info) const {
  BOOST_ASSERT(!workers_.empty());
  return hash_value(info) % workers_.size();
}

}  // namespace rpcz
