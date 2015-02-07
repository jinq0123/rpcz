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

#include <rpcz/requester.hpp>

#include <boost/make_shared.hpp>
#include <rpcz/dealer_connection.hpp>
#include <rpcz/invalid_message_error.hpp>
#include <rpcz/logging.hpp>  // for CHECK_EQ
#include <rpcz/rpc_controller.hpp>
#include <rpcz/rpc_error.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_request_header
#include <rpcz/sync_call_handler.hpp>
#include <rpcz/zmq_utils.hpp>  // for string_to_message()

namespace rpcz {

requester::requester(const dealer_connection& conn)
    : dealer_conn_(new dealer_connection(conn)) {  // shared_ptr
}

requester::~requester() {
}

// XXX
//void requester::call_method_full(
//    const std::string& service_name,
//    const std::string& method_name,
//    const ::google::protobuf::Message* request_msg,
//    const std::string& request,
//    ::google::protobuf::Message* response_msg,
//    std::string* response_str,
//    rpc_controller* rpc_controller,
//    closure* done) {
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
//  // rpc_controller will be deleted on response or timeout.
//  rpc_controller* ctrl = new rpc_controller(
//      response_message_handler(), error_handler(), -1);
//  connection_.send_request(msg_vector, ctrl);
//}

// XXX
//void requester::call_method0(const std::string& service_name,
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
//void requester::call_method(
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

void requester::request(
    const google::protobuf::MethodDescriptor& method,
    const google::protobuf::Message& request,
    const response_message_handler& handler,
    long timeout_ms) {
  rpc_header rpc_hdr;
  rpc_request_header* req_hdr = rpc_hdr.mutable_req_hdr();
  req_hdr->set_service(method.service()->name());
  req_hdr->set_method(method.name());

  size_t msg_size = rpc_hdr.ByteSize();
  scoped_ptr<zmq::message_t> msg_out(new zmq::message_t(msg_size));
  CHECK(rpc_hdr.SerializeToArray(msg_out->data(), msg_size));

  scoped_ptr<zmq::message_t> payload_out;
  size_t bytes = request.ByteSize();
  payload_out.reset(new zmq::message_t(bytes));
  if (!request.SerializeToArray(payload_out->data(), bytes)) {
    throw invalid_message_error("Request serialization failed.");  // XXX
  }

  message_vector msg_vector;
  msg_vector.push_back(msg_out.release());
  msg_vector.push_back(payload_out.release());

  // rpc_controller will be deleted on response or timeout.
  // rpc_controller deleted in worker_thread_fun().
  // XXX delete on timeout.
  rpc_controller* ctrl = new rpc_controller(handler, timeout_ms);
  dealer_conn_->send_request(msg_vector, ctrl);
}

channel_ptr requester::make_shared(const dealer_connection& conn) {
  return boost::make_shared<requester>(conn);
}

channel_ptr requester::make_shared(const std::string& endpoint) {
  return make_shared(dealer_connection(endpoint));
}

}  // namespace rpcz