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

#include <rpcz/broker_thread.hpp>

#include <rpcz/callback.hpp>
#include <rpcz/clock.hpp>  // for zclock_ms()
#include <rpcz/internal_commands.hpp>
#include <rpcz/logging.hpp>
#include <rpcz/rpc_controller.hpp>  // for rpc_controller
#include <rpcz/sync_event.hpp>
#include <rpcz/zmq_utils.hpp>

namespace rpcz {

broker_thread::broker_thread(
    zmq::context_t& context, int nthreads, sync_event* ready_event,
    zmq::socket_t* frontend_socket)
    : context_(context),
      frontend_socket_(frontend_socket),
      current_worker_(0) {
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
    CHECK_EQ(kReady, command) << "Got unexpected command " << (int)command;
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
  switch (command) {
    case kQuit:
      // Ask the workers to quit. They'll in turn send kWorkerDone.
      for (size_t i = 0; i < workers_.size(); ++i) {
        send_string(frontend_socket_, workers_[i], ZMQ_SNDMORE);
        send_empty_message(frontend_socket_, ZMQ_SNDMORE);
        send_char(frontend_socket_, kWorkerQuit, 0);
      }
      break;
    case kConnect:
      handle_connect_command(sender, message_to_string(iter.next()));
      break;
    case kBind: {
      std::string endpoint(message_to_string(iter.next()));
      const service_factory_map* factories
          = interpret_message<const service_factory_map*>(iter.next());
      assert(factories);
      handle_bind_command(sender, endpoint, *factories);
      break;
    }
    case kUnbind: {
      std::string endpoint(message_to_string(iter.next()));
      handle_unbind_command(sender, endpoint);
      break;
    }
    case kRequest:
      send_request(iter);
      break;
    case kReply:
      send_reply(iter);
      break;
    case kReady:
      CHECK(false);
      break;
    case kWorkerDone:
      workers_.erase(std::remove(workers_.begin(), workers_.end(), sender));
      current_worker_ = 0;
      if (workers_.size() == 0) {
        // All workers are gone, time to quit.
        reactor_.set_should_quit();
      }
      break;
    case kRunClosure:
      add_closure(interpret_message<closure*>(iter.next()));
      break;
  }  // switch
}

void broker_thread::begin_worker_command(char command) {
  send_string(frontend_socket_, workers_[current_worker_], ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_char(frontend_socket_, command, ZMQ_SNDMORE);
  ++current_worker_;
  if (current_worker_ == workers_.size()) {
    current_worker_ = 0;
  }
}

void broker_thread::add_closure(closure* closure) {
  begin_worker_command(kRunClosure);
  send_pointer(frontend_socket_, closure, 0);
}

void broker_thread::handle_connect_command(
      const std::string& sender, const std::string& endpoint) {
  zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_DEALER);
  client_sockets_.push_back(socket);
  int linger_ms = 0;
  socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  socket->connect(endpoint.c_str());
  reactor_.add_socket(socket, new_permanent_callback(
          this, &broker_thread::handle_client_socket,
          socket));

  send_string(frontend_socket_, sender, ZMQ_SNDMORE);
  send_empty_message(frontend_socket_, ZMQ_SNDMORE);
  send_uint64(frontend_socket_, client_sockets_.size() - 1, 0);
}

void broker_thread::handle_bind_command(
    const std::string& sender,
    const std::string& endpoint,
    const service_factory_map& factories) {
  zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_ROUTER);  // delete in reactor
  int linger_ms = 0;
  socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  socket->bind(endpoint.c_str());  // TODO: catch exception
  uint64 server_socket_idx = server_sockets_.size();
  server_sockets_.push_back(socket);
  bind_map_[endpoint] = socket;  // for unbind
  // reactor will own socket and callback.
  reactor_.add_socket(socket, new_permanent_callback(
      this, &broker_thread::handle_server_socket,
      server_socket_idx, &factories));

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

void broker_thread::handle_socket_deleted(const std::string sender) {
    send_string(frontend_socket_, sender, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, 0);
}

void broker_thread::handle_server_socket(uint64 server_socket_idx,
    const service_factory_map* factories) {
  assert(NULL != factories);
  message_iterator iter(*server_sockets_[server_socket_idx]);
  std::string sender(message_to_string(iter.next()));
  if (iter.next().size() != 0) return;
  request_handler* handler = request_handler_manager_
      .get_handler(sender, *factories, server_socket_idx);
  assert(NULL != handler);
  begin_worker_command(kHandleRequest);
  send_pointer(frontend_socket_, handler, ZMQ_SNDMORE);
  forward_messages(iter, *frontend_socket_);
}

void broker_thread::send_request(message_iterator& iter) {
  uint64 connection_id = interpret_message<uint64>(iter.next());
  rpc_controller* ctrl = interpret_message<rpc_controller*>(iter.next());
  BOOST_ASSERT(ctrl);
  event_id event_id = event_id_generator_.get_next();
  remote_response_map_[event_id] = ctrl;
  int64 timeout_ms = ctrl->get_timeout_ms();
  if (-1 != timeout_ms) {
    // XXX when to delete timeout handler?
    reactor_.run_closure_at(zclock_ms() + timeout_ms,
        new_callback(this, &broker_thread::handle_timeout, event_id));
  }
  zmq::socket_t* socket = client_sockets_[connection_id];
  send_string(socket, "", ZMQ_SNDMORE);
  send_uint64(socket, event_id, ZMQ_SNDMORE);
  forward_messages(iter, *socket);
}

void broker_thread::handle_client_socket(zmq::socket_t* socket) {
  message_iterator iter(*socket);
  if (iter.next().size() != 0) {
    return;
  }
  if (!iter.has_more()) {
    return;
  }
  event_id event_id(interpret_message<event_id>(iter.next()));
  remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
  if (response_iter == remote_response_map_.end()) {
    return;
  }
  const rpc_controller* ctrl = response_iter->second;
  BOOST_ASSERT(ctrl);
  begin_worker_command(kHandleResponse);
  send_pointer(frontend_socket_, ctrl, ZMQ_SNDMORE);
  forward_messages(iter, *frontend_socket_);
  remote_response_map_.erase(response_iter);
}

void broker_thread::handle_timeout(event_id event_id) {
  remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
  if (response_iter == remote_response_map_.end()) {
      return;
  }
  rpc_controller* ctrl = response_iter->second;
  BOOST_ASSERT(ctrl);
  ctrl->set_timeout_expired();
  begin_worker_command(kHandleResponse);
  send_pointer(frontend_socket_, ctrl, 0);
  remote_response_map_.erase(response_iter);
}

void broker_thread::send_reply(message_iterator& iter) {
  uint64 server_socket_idx = interpret_message<uint64>(iter.next());
  zmq::socket_t* socket = server_sockets_[server_socket_idx];
  forward_messages(iter, *socket);
}

}  // namespace rpcz
