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

#include "rpc_channel_impl.hpp"

#include "logging.hpp"  // for CHECK_EQ
#include "rpc_context.hpp"
#include "rpcz/invalid_message_error.hpp"
#include "rpcz/rpc_error.hpp"
#include "sync_call_handler.hpp"
#include "zmq_utils.hpp"  // for string_to_message()

namespace rpcz {

rpc_channel_impl::rpc_channel_impl(connection connection)
    : connection_(connection) {
}

rpc_channel_impl::~rpc_channel_impl() {
}

// XXX
//void rpc_channel_impl::call_method_full(
//    const std::string& service_name,
//    const std::string& method_name,
//    const ::google::protobuf::Message* request_msg,
//    const std::string& request,
//    ::google::protobuf::Message* response_msg,
//    std::string* response_str,
//    rpc_controller* rpc_controller,
//    closure* done) {
//  CHECK_EQ(rpc_controller->get_status(), status::INACTIVE);
//  rpc_request_header generic_request;
//  generic_request.set_service(service_name);
//  generic_request.set_method(method_name);
//
//  size_t msg_size = generic_request.ByteSize();
//  scoped_ptr<zmq::message_t> msg_out(new zmq::message_t(msg_size));
//  CHECK(generic_request.SerializeToArray(msg_out->data(), msg_size));
//
//  scoped_ptr<zmq::message_t> payload_out;
//  if (request_msg != NULL) {
//    size_t bytes = request_msg->ByteSize();
//    payload_out.reset(new zmq::message_t(bytes));
//    if (!request_msg->SerializeToArray(payload_out->data(), bytes)) {
//      throw invalid_message_error("Request serialization failed.");
//    }
//  } else {
//    payload_out.reset(string_to_message(request));
//  }
//
//  message_vector msg_vector;
//  msg_vector.push_back(msg_out.release());
//  msg_vector.push_back(payload_out.release());
//
//  // XXX Merge rpc_context and rpc_controller.
//  // rpc_context will be deleted on response or timeout.
//  rpc_context* ctx = new rpc_context(
//      response_message_handler(), error_handler(), -1);  // XXX
//  // XXX ctx->rpc_controller = rpc_controller;
//  // XXX ctx->user_closure = done;
//  // DEL ctx->response_str = response_str;
//  // DEL ctx->response_msg = response_msg;
//  rpc_controller->set_status(status::ACTIVE);
//  connection_.send_request(msg_vector, ctx);
//}

// XXX
//void rpc_channel_impl::call_method0(const std::string& service_name,
//                                const std::string& method_name,
//                                const std::string& request,
//                                std::string* response,
//                                rpc_controller* rpc_controller,
//                                closure* done) {
//  call_method_full(service_name,
//                 method_name,
//                 NULL,
//                 request,
//                 NULL,
//                 response,
//                 rpc_controller,
//                 done);
//}

// XXX
//void rpc_channel_impl::call_method(
//    const std::string& service_name,
//    const google::protobuf::MethodDescriptor* method,
//    const google::protobuf::Message& request,
//    google::protobuf::Message* response,
//    rpc_controller* rpc_controller,
//    closure* done) {
//  call_method_full(service_name,
//                 method->name(),
//                 &request,
//                 "",
//                 response,
//                 NULL,
//                 rpc_controller,
//                 done);
//}

// XXX rename other call_method() to async_call().
// XXX Add sync_call().
void rpc_channel_impl::async_call(
    const std::string& service_name,
    const google::protobuf::MethodDescriptor* method,
    const google::protobuf::Message& request,
    const response_message_handler& handler,
    long deadline_ms) {
  // XXX CHECK_EQ(rpc_controller->get_status(), status::INACTIVE);
  rpc_request_header generic_request;
  generic_request.set_service(service_name);
  generic_request.set_method(method->name());

  size_t msg_size = generic_request.ByteSize();
  scoped_ptr<zmq::message_t> msg_out(new zmq::message_t(msg_size));
  CHECK(generic_request.SerializeToArray(msg_out->data(), msg_size));

  scoped_ptr<zmq::message_t> payload_out;
  size_t bytes = request.ByteSize();
  payload_out.reset(new zmq::message_t(bytes));
  if (!request.SerializeToArray(payload_out->data(), bytes)) {
    throw invalid_message_error("Request serialization failed.");  // XXX
  }

  message_vector msg_vector;
  msg_vector.push_back(msg_out.release());
  msg_vector.push_back(payload_out.release());

  // rpc_context will be deleted on response or timeout.
  // rpc_context deleted in worker_thread_fun().
  // XXX delete on timeout.
  rpc_context* ctx = new rpc_context(handler, deadline_ms);
  connection_.send_request(msg_vector, ctx);
}

void rpc_channel_impl::sync_call(
    const std::string& service_name,
    const google::protobuf::MethodDescriptor* method,
    const google::protobuf::Message& request,
    long deadline_ms,
    google::protobuf::Message* response) {
  sync_call_handler handler(response);
  async_call(service_name, method, request,
      handler, deadline_ms);
  handler.wait();
  rpc_error* err = handler.get_rpc_error();
  if (err)
    throw *err;
}

}  // namespace rpcz
