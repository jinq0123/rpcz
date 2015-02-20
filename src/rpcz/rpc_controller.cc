// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/rpc_controller.hpp>

#include <rpcz/application_error_code.hpp>  // for error_code
#include <rpcz/rpc_error.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_response_header
#include <rpcz/zmq_utils.hpp>  // for message_iterator

namespace rpcz {

// Run in worker thread.
    // XXXX
//void rpc_controller::handle_response(
//    message_iterator& iter) {
//  if (!timeout_expired_) {
//    // in most cases
//    handle_done_response(iter);
//    return;
//  }
//  handle_timeout_expired();
//}  // handle_response()

void rpc_controller::handle_timeout_expired() {
  handle_error(error_code::TIMEOUT_EXPIRED, "");
}

void rpc_controller::handle_error(
    int error_code, const std::string& error_str) {
  if (handler_.empty()) return;
  rpc_error e(error_code, error_str);
  handler_(&e, NULL, 0);
}

//inline void rpc_controller::handle_done_response(message_iterator& iter) {
//  if (!iter.has_more()) {
//    handle_error(error_code::INVALID_MESSAGE, "");
//    return;
//  }
  // XXXX DEL
  //rpc_header rpc_hdr;
  //zmq::message_t& msg_in = iter.next();
  //if (!rpc_hdr.ParseFromArray(msg_in.data(), msg_in.size())) {
  //  handle_error(error_code::INVALID_MESSAGE, "");
  //  return;
  //}
  //if (!rpc_hdr.has_resp_hdr()) {
  //  handle_error(error_code::INVALID_MESSAGE, "No response header.");
  //  return;
  //}
  //const rpc_response_header& resp_hdr = rpc_hdr.resp_hdr();
  //if (resp_hdr.has_error_code()) {
  //  handle_error(resp_hdr.error_code(), resp_hdr.error_str());
  //  return;
  //}
  //if (!iter.has_more()) {
  //  handle_error(error_code::INVALID_MESSAGE, "");
  //  return;
  //}

  //// the most case
  //const zmq::message_t& payload = iter.next();
  //handle_response_message(payload.data(), payload.size());
//}  // handle_done_response()

}  // namespace rpcz
