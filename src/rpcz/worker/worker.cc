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

// Worker thread function.

#include <rpcz/worker/worker.hpp>

#include <zmq.hpp>  // for message_t

#include <rpcz/callback.hpp>  // for closure
#include <rpcz/request_handler.hpp>
#include <rpcz/rpc_controller.hpp>  // for rpc_controller
#include <rpcz/rpcz.pb.h>
#include <rpcz/worker/worker_cmd.hpp>  // for kRunClosure

namespace rpcz {

worker::worker(size_t worker_index)
    : worker_index_(worker_index),
      cmd_queue_(new worker_cmd_queue) {
}

worker::~worker() {
}

void worker::operator()() {
  worker_cmd_queue& cmd_queue = *cmd_queue_;
  worker_cmd_ptr cmd;
  bool should_continue = true;
  do {
    cmd_queue.pop(cmd);  // blocking
    BOOST_ASSERT(cmd);
    using namespace b2w;  // broker to worker command
    switch (cmd->cmd) {
      case kQuitWorker:
        should_continue = false;
        break;
      case kRunClosure:
        run_closure(cmd);
        break;
      case kStartRpc:
        start_rpc(cmd);
        break;
      case kHandleData:
        handle_data(cmd);
        break;
      case kHandleTimeout:
        handle_timeout(cmd);
        break;
      case kRegisterSvc:
        register_service(cmd);
        break;
      default:
        BOOST_ASSERT(false);
        break;
    }  // switch
  } while (should_continue);
}

worker_cmd_queue_ptr worker::get_cmd_queue() const {
  return cmd_queue_;
}

inline void worker::run_closure(const worker_cmd_ptr& cmd) {
  b2w::run_closure_cmd* run_cmd = static_cast<b2w::run_closure_cmd*>(cmd.get());
  closure* clsr = run_cmd->clsr;
  BOOST_ASSERT(clsr);
  clsr->run();
}

inline void worker::start_rpc(const worker_cmd_ptr& cmd) {
  b2w::start_rpc_cmd* start = static_cast<b2w::start_rpc_cmd*>(cmd.get());
  rpc_controller* ctrl = start->ctrl;
  BOOST_ASSERT(ctrl);
  uint64 event_id = ctrl->get_event_id();
  remote_response_map_[event_id] = ctrl;
}

inline void worker::handle_data(const worker_cmd_ptr& cmd) {
  b2w::handle_data_cmd* hd_cmd = static_cast<b2w::handle_data_cmd*>(cmd.get());
  const zmq::message_t& header = hd_cmd->header;
  rpc_header rpc_hdr;
  if (!rpc_hdr.ParseFromArray(header.data(), header.size())) {
    // DLOG(INFO) << "Received bad header.";
    return;
  }
  if (rpc_hdr.has_req_hdr()) {
    handle_request(rpc_hdr.req_hdr(), *hd_cmd);
    return;
  }
  if (rpc_hdr.has_resp_hdr()) {
    handle_response(rpc_hdr.resp_hdr(), *hd_cmd);
    return;
  }
}

void worker::handle_timeout(const worker_cmd_ptr& cmd) {
  const b2w::handle_timeout_cmd* timeout_cmd = static_cast<
      const b2w::handle_timeout_cmd*>(cmd.get());
  uint64 event_id = timeout_cmd->event_id;
  remote_response_map::iterator response_iter
      = remote_response_map_.find(event_id);
  if (response_iter == remote_response_map_.end())
    return;  // already be responded?

  rpc_controller* ctrl = response_iter->second;
  BOOST_ASSERT(ctrl);
  BOOST_ASSERT(ctrl->get_event_id() == event_id);
  ctrl->handle_timeout();
  delete ctrl;
  remote_response_map_.erase(response_iter);
}

void worker::register_service(const worker_cmd_ptr& cmd) {
  const b2w::register_svc_cmd* reg = static_cast<
      const b2w::register_svc_cmd*>(cmd.get());
  request_handler& handler = request_handler_map_.get_handler(*reg->info);
  handler.register_service(reg->name, reg->svc);
}

inline void worker::handle_request(
    const rpc_request_header& req_hdr,
    const b2w::handle_data_cmd& cmd) {
  BOOST_ASSERT(cmd.info);
  const connection_info& conn_info = *cmd.info;
  const zmq::message_t& payload = cmd.payload;
  request_handler& handler = request_handler_map_.get_handler(conn_info);
  handler.handle_request(req_hdr, payload.data(), payload.size());
}

void worker::handle_response(
    const rpc_response_header& resp_hdr,
    const b2w::handle_data_cmd& cmd) {
  if (resp_hdr.has_error_code()) {
    handle_error_resp(resp_hdr);
    return;
  }

  // normal response
  const zmq::message_t& payload = cmd.payload;
  handle_done_resp(resp_hdr.event_id(), payload);
}

inline void worker::handle_done_resp(uint64 event_id,
    const zmq::message_t& response) {
  remote_response_map::iterator response_iter
      = remote_response_map_.find(event_id);
  if (response_iter == remote_response_map_.end())
    return;  // maybe already timeout

  rpc_controller* ctrl = response_iter->second;
  BOOST_ASSERT(ctrl);
  BOOST_ASSERT(ctrl->get_event_id() == event_id);
  ctrl->handle_response(response.data(), response.size());
  delete ctrl;
  remote_response_map_.erase(response_iter);
}

void worker::handle_error_resp(const rpc_response_header& resp_hdr) {
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
