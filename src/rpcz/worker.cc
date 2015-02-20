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

#include <rpcz/worker.hpp>

#include <zmq.hpp>

#include <rpcz/callback.hpp>  // for closure
#include <rpcz/connection_info_zmq.hpp>  // for read_connection_info()
#include <rpcz/internal_commands.hpp>  // for kReady
#include <rpcz/logging.hpp>  // for CHECK_EQ()
#include <rpcz/request_handler.hpp>
#include <rpcz/rpc_controller.hpp>  // for rpc_controller
#include <rpcz/rpcz.pb.h>
#include <rpcz/zmq_utils.hpp>  // for send_empty_message()

namespace rpcz {

worker::worker(size_t worker_index,
               const std::string& frontend_endpoint,
               zmq::context_t& context)
    : worker_index_(worker_index),
      frontend_endpoint_(frontend_endpoint),
      context_(context) {
}

worker::~worker() {
}

void worker::operator()() {
  zmq::socket_t socket(context_, ZMQ_DEALER);
  socket.connect(frontend_endpoint_.c_str());
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kWorkerReady, ZMQ_SNDMORE);
  send_uint64(&socket, worker_index_);
  bool should_continue = true;
  do {
    message_iterator iter(socket);
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    using namespace b2w;  // broker to worker command
    switch (command) {
      case kWorkerQuit:
        should_continue = false;
        break;
      case kRunClosure:
        interpret_message<closure*>(iter.next())->run();
        break;
      case kStartRpc:
        start_rpc(iter);
        break;
      case kHandleData:
        handle_data(socket);
        break;
      case kHandleTimeout:
        handle_timeout(iter);
        break;
      default:
        CHECK(false);
        break;
    }  // switch
  } while (should_continue);
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kWorkerDone);
}

void worker::start_rpc(message_iterator& iter) {
  BOOST_ASSERT(iter.has_more());
  rpc_controller* ctrl = interpret_message<rpc_controller*>(iter.next());
  BOOST_ASSERT(!iter.has_more());
  BOOST_ASSERT(ctrl);
  uint64 event_id = ctrl->get_event_id();
  remote_response_map_[event_id] = ctrl;
}

void worker::handle_data(zmq::socket_t& socket) {
  connection_info info;
  read_connection_info(&socket, &info);
  message_iterator iter(socket);
  if (!iter.has_more()) return;
  const zmq::message_t& msg = iter.next();
  rpc_header rpc_hdr;
  if (!rpc_hdr.ParseFromArray(msg.data(), msg.size())) {
    // Handle bad rpc.
    DLOG(INFO) << "Received bad header.";
    // XXXX rep.reply_error(error_code::INVALID_HEADER, "Invalid rpc_header.");
    return;
  }
  if (rpc_hdr.has_req_hdr()) {
    handle_request(rpc_hdr.req_hdr(), iter);
    return;
  }
  if (rpc_hdr.has_resp_hdr()) {
    handle_response(rpc_hdr.resp_hdr(), iter);
    return;
  }
  // TODO: disconnect
}

  // XXXX
          // XXX
        //request_handler* handler =
        //    interpret_message<request_handler*>(iter.next());
        //assert(handler);
        //handler->handle_request(iter);

void worker::handle_timeout(message_iterator& iter) {
  uint64 event_id = interpret_message<uint64>(iter.next());
  // XXXX
  //remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
  //if (response_iter == remote_response_map_.end()) {
  //  return;
  //}
  //rpc_controller* ctrl = response_iter->second;
  //BOOST_ASSERT(ctrl);
  //ctrl->set_timeout_expired();  XXX ->timeout()
  //remote_response_map_.erase(response_iter);
}

void worker::handle_request(
    const ::rpcz::rpc_request_header& req_hdr,
    message_iterator& iter) {
  if (!iter.has_more()) return;
        // XXXX
}

void worker::handle_response(
    const ::rpcz::rpc_response_header& resp_hdr,
    message_iterator& iter) {
  // Must read until end.
  bool has_error = resp_hdr.has_error_code();
  if (has_error) {
    handle_error_resp(resp_hdr);
    return;
  }

  if (!iter.has_more()) {
    // XXX handle_error(error_code::INVALID_MESSAGE, "");
    return;  // Illegal response.
  }
  // normal response
  const zmq::message_t& payload = iter.next();
  handle_done_response(resp_hdr.event_id(), payload);
}

void worker::handle_done_response(uint64 event_id,
    const zmq::message_t& response) {
  remote_response_map::iterator response_iter
      = remote_response_map_.find(event_id);
  if (response_iter == remote_response_map_.end())
    return;  // maybe already timedout

  rpc_controller* ctrl = response_iter->second;
  BOOST_ASSERT(ctrl);
  BOOST_ASSERT(ctrl->get_event_id() == event_id);
  ctrl->handle_response(response.data(), response.size());
  delete ctrl;
  remote_response_map_.erase(response_iter);
}

void worker::handle_error_resp(const ::rpcz::rpc_response_header& resp_hdr) {
  BOOST_ASSERT(resp_hdr.has_error_code());
  remote_response_map::iterator response_iter
      = remote_response_map_.find(resp_hdr.event_id());
  if (response_iter == remote_response_map_.end())
    return;  // maybe already timedout

  rpc_controller* ctrl = response_iter->second;
  BOOST_ASSERT(ctrl);
  BOOST_ASSERT(ctrl->get_event_id() == resp_hdr.event_id());
  ctrl->handle_error(resp_hdr.error_code(), resp_hdr.error_str());
  delete ctrl;
  remote_response_map_.erase(response_iter);
}

}  // namespace rpcz
