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
#include <rpcz/connection_info_hash.hpp>  // for hash_value()
#include <rpcz/connection_info_zmq.hpp>  // for read_connection_info()
#include <rpcz/internal_commands.hpp>
#include <rpcz/logging.hpp>
#include <rpcz/rpc_controller.hpp>  // for rpc_controller
#include <rpcz/sync_event.hpp>
#include <rpcz/utc_ms.hpp>
#include <rpcz/worker/workers_commander.hpp>
#include <rpcz/zmq_utils.hpp>

namespace rpcz {

broker_thread::broker_thread(
    zmq::context_t& context,
    sync_event* ready_event,
    zmq::socket_t* frontend_socket,
    const workers_commander_ptr& wkrs_cmdr)
    : context_(context),
      workers_(wkrs_cmdr->get_workers()),
      frontend_socket_(frontend_socket),
      workers_commander_(wkrs_cmdr) {
  BOOST_ASSERT(workers_ > 0);
  BOOST_ASSERT(wkrs_cmdr);
  // Index 0 is reserved for debug check.
  dealer_sockets_.push_back(NULL);
  BOOST_ASSERT(1 == dealer_sockets_.size());
  router_sockets_.push_back(NULL);
  BOOST_ASSERT(1 == router_sockets_.size());

  ready_event->signal();
  reactor_.add_socket(frontend_socket, new_permanent_callback(
      this, &broker_thread::handle_frontend_socket,
      frontend_socket));
}

void broker_thread::run(
    zmq::context_t& context,
    sync_event* ready_event,
    zmq::socket_t* frontend_socket,
    const workers_commander_ptr& workers_commander) {
  BOOST_ASSERT(workers_commander);
  broker_thread bt(context, ready_event,
      frontend_socket, workers_commander);
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
      handle_quit_command();
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

    default:
      BOOST_ASSERT(false);
      break;
  }  // switch
}

// Add closure to random worker thread.
inline void broker_thread::add_closure(closure* closure) {
  BOOST_ASSERT(workers_ > 0);
  size_t worker_index = rand() % workers_;
  workers_commander_->run_closure(worker_index, closure);
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
  int send_hwm = 1000 * 1000;
  socket->setsockopt(ZMQ_SNDHWM, &send_hwm, sizeof(send_hwm));
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
  BOOST_ASSERT((*it).second);
  reactor_.del_socket((*it).second,
      new_callback(this, &broker_thread::handle_socket_deleted, std::string(sender)));
  bind_map_.erase(it);
  // Socket is not delelted yet.
  // It will callback on deleted before next zmq_poll().
}

void broker_thread::handle_quit_command() {
  reactor_.set_should_quit();
}

void broker_thread::handle_socket_deleted(const std::string sender) {
  send_string(frontend_socket_, sender, ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, 0);  // Only to end zmq message?
}

inline void broker_thread::handle_router_socket(uint64 router_index) {
  BOOST_ASSERT(is_router_index_legal(router_index));
  message_iterator iter(*router_sockets_[router_index]);
  std::string sender(message_to_string(iter.next()));
  if (!iter.has_more()) return;
  connection_info_ptr info(new connection_info(router_index, sender));  // shared_ptr
  handle_socket(info, iter);
}

inline void broker_thread::handle_dealer_socket(uint64 dealer_index) {
  BOOST_ASSERT(is_dealer_index_legal(dealer_index));
  message_iterator iter(*dealer_sockets_[dealer_index]);
  connection_info_ptr info(new connection_info(dealer_index));  // shared_ptr
  handle_socket(info, iter);
}

inline void broker_thread::handle_socket(
    const connection_info_ptr& info, message_iterator& iter) {
  BOOST_ASSERT(info);
  if (iter.next().size() != 0) return;
  if (!iter.has_more()) return;
  handle_data_cmd_ptr cmd_ptr(new b2w::handle_data_cmd);  // shared_ptr
  b2w::handle_data_cmd& cmd = *cmd_ptr;
  cmd.header.move(&iter.next());
  if (iter.has_more())
    cmd.payload.move(&iter.next());
  cmd.info = info;
  workers_commander_->handle_data(get_worker_index(*info), cmd_ptr);
}

void broker_thread::handle_timeout(uint64 event_id, size_t worker_index) {
  BOOST_ASSERT(workers_);
  worker_index %= workers_;
  workers_commander_->handle_timeout(worker_index, event_id);
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
    reactor_.run_closure_at(utc_ms() + timeout_ms,
        new_callback(this, &broker_thread::handle_timeout,
            event_id, get_worker_index(info)));
  }
  forward_to(info, iter);  // Request remote server.

  start_rpc(info, ctrl);  // Let worker thread start this rpc
}

inline void broker_thread::start_rpc(
    const connection_info& info,
    rpc_controller* ctrl) {
  workers_commander_->start_rpc(get_worker_index(info), ctrl);
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
  connection_info_ptr info(new connection_info);
  read_connection_info(iter, info.get());
  BOOST_ASSERT(is_connection_info_legal(*info));
  BOOST_ASSERT(iter.has_more());
  std::string name = message_to_string(iter.next());
  BOOST_ASSERT(iter.has_more());
  iservice* svc = interpret_message<iservice*>(iter.next());
  BOOST_ASSERT(!iter.has_more());
  BOOST_ASSERT(svc);
  workers_commander_->register_svc(get_worker_index(*info),
                                   info, name, svc);
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
  BOOST_ASSERT(workers_);
  return hash_value(info) % workers_;
}

}  // namespace rpcz
