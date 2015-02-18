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
      frontend_socket_(frontend_socket),
      current_worker_(0) {
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
  for (int i = 0; i < nthreads; ++i) {
    message_iterator iter(*frontend_socket_);
    std::string sender = message_to_string(iter.next());
    assert(!sender.empty());  // zmq id
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    CHECK_EQ(c2b::kWorkerReady, command)
        << "Got unexpected command " << (int)command;
    workers_.push_back(sender);
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

    // XXX Request from dealer (dealer index)
    // XXX Request from router (router index, sender)
    case kRequest:
      send_request(frontend_socket);
      break;
    case kReply:
      send_reply(frontend_socket);
      break;

    case kRunClosure:
      add_closure(interpret_message<closure*>(iter.next()));
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

// XXX Use worker name to specify worker thread...
// command must be broker to worker (b2w) command.
void broker_thread::begin_worker_command(char command) {
  send_string(frontend_socket_, workers_[current_worker_], ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_char(frontend_socket_, command, ZMQ_SNDMORE);
  ++current_worker_;
  if (current_worker_ == workers_.size()) {
    current_worker_ = 0;
  }
}

inline void broker_thread::add_closure(closure* closure) {
  begin_worker_command(b2w::kRunClosure);
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
  send_empty_message(frontend_socket_, 0);
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
  current_worker_ = 0;  // XXX ?
  if (!workers_.empty())
    return;
  // All workers are gone, time to quit.
  reactor_.set_should_quit();
}

void broker_thread::handle_socket_deleted(const std::string sender) {
    send_string(frontend_socket_, sender, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, 0);
}

// XXX merge handle_router_socket() and handle_dealer_socket()

void broker_thread::handle_router_socket(uint64 router_index) {
  BOOST_ASSERT(is_router_index_legal(router_index));
  message_iterator iter(*router_sockets_[router_index]);

  // XXX
  //std::string sender(message_to_string(iter.next()));
  //if (iter.next().size() != 0) return;
  //// XXX request_handler_manager_ is binded to router. Rename it to router_handler_manager?
  //request_handler* handler = request_handler_manager_
  //    .get_handler(sender, *factories, router_index);
  //assert(NULL != handler);
  begin_worker_command(b2w::kHandleRouterData);
  send_uint64(frontend_socket_, router_index, ZMQ_SNDMORE);
  forward_messages(iter, *frontend_socket_);
}

void broker_thread::handle_dealer_socket(uint64 dealer_index) {
  BOOST_ASSERT(is_dealer_index_legal(dealer_index));
  message_iterator iter(*dealer_sockets_[dealer_index]);
  if (iter.next().size() != 0) {
    return;
  }
  if (!iter.has_more()) {
    return;
  }
  // XXX no event id ...
  //event_id event_id(interpret_message<event_id>(iter.next()));
  //remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
  //if (response_iter == remote_response_map_.end()) {
  //  return;
  //}
  //const rpc_controller* ctrl = response_iter->second;
  //BOOST_ASSERT(ctrl);
  begin_worker_command(b2w::kHandleDealerData);
  //send_pointer(frontend_socket_, ctrl, ZMQ_SNDMORE);
  forward_messages(iter, *frontend_socket_);
  //remote_response_map_.erase(response_iter);
  // XXX move remote_response_map_ to manager and make it thread-safe
}

void broker_thread::handle_timeout(uint64 event_id) {
  begin_worker_command(b2w::kHandleTimeout);
  send_uint64(frontend_socket_, event_id, 0);
}

// XXX Map connection to worker thread.
// XXX dealer index -> worker thread index (worker name)
// XXX (router index, sender) -> worker thread index (worker name)

inline void broker_thread::send_request(zmq::socket_t* frontend_socket) {
  BOOST_ASSERT(frontend_socket);
  connection_info info;
  read_connection_info(frontend_socket, &info);

  // XXX send request...
  //uint64 dealer_index = interpret_message<uint64>(iter.next());
  //BOOST_ASSERT(is_dealer_index_legal(dealer_index));
  //rpc_controller* ctrl = interpret_message<rpc_controller*>(iter.next());
  //BOOST_ASSERT(ctrl);
  //uint64 event_id = ctrl->get_event_id();
  //remote_response_map_[event_id] = ctrl;

  //int64 timeout_ms = ctrl->get_timeout_ms();
  //if (-1 != timeout_ms) {
  //  // XXX when to delete timeout handler?
  //  reactor_.run_closure_at(zclock_ms() + timeout_ms,
  //      new_callback(this, &broker_thread::handle_timeout, event_id));
  //}

  //zmq::socket_t* socket = dealer_sockets_[dealer_index];
  //BOOST_ASSERT(socket);
  //send_string(socket, "", ZMQ_SNDMORE);
  //forward_messages(iter, *socket);
}

inline void broker_thread::send_reply(zmq::socket_t* frontend_socket) {
  BOOST_ASSERT(frontend_socket);
  connection_info info;
  read_connection_info(frontend_socket, &info);

  // XXX send reply...
  //uint64 router_index = interpret_message<uint64>(iter.next());
  //BOOST_ASSERT(is_router_index_legal(router_index));
  //zmq::socket_t* socket = router_sockets_[router_index];
  //BOOST_ASSERT(socket);
  //forward_messages(iter, *socket);
}

bool broker_thread::is_dealer_index_legal(uint64 dealer_index) const {
  // Index 0 is reserved.
  return dealer_index > 0 && dealer_index < dealer_sockets_.size();
}

bool broker_thread::is_router_index_legal(uint64 router_index) const {
  // Index 0 is reserved.
  return router_index > 0 && router_index < router_sockets_.size();
}

}  // namespace rpcz
