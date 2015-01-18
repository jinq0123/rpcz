// Licensed under the Apache License, Version 2.0 (the "License");
// Author: nadavs@google.com <Nadav Samet>
//         Jin Qing (http://blog.csdn.net/jq0123)
//
// Handle rpc response. Call response handler or error handler.
// Run in worker thread.

#ifndef RPCZ_HANDLE_RESPONSE_HPP
#define RPCZ_HANDLE_RESPONSE_HPP

#include "connection_manager_status.hpp"  // for connection_manager_status
#include "logging.hpp"  // for CHECK()
#include "rpc_context.hpp"  // for rpc_context
#include "rpcz/application_error_code.hpp"  // for application_error
#include "zmq_utils.hpp"  // for message_iterator

namespace rpcz {

namespace detail {

inline void handle_done_response(
    rpc_context& context,
    message_iterator& iter) {
  if (!iter.has_more()) {
    context.handle_application_error(application_error::INVALID_MESSAGE, "");
    return;
  }
  rpc_response_header generic_response;
  zmq::message_t& msg_in = iter.next();
  if (!generic_response.ParseFromArray(msg_in.data(), msg_in.size())) {
    context.handle_application_error(application_error::INVALID_MESSAGE, "");
    return;
  }
  if (generic_response.status() != status::OK) {
    context.handle_application_error(
        generic_response.application_error(),
        generic_response.error());
    return;
  }

  if (!iter.has_more()) {
    context.handle_application_error(application_error::INVALID_MESSAGE, "");
    return;
  }

  zmq::message_t& payload = iter.next();
  context.handle_response_message(payload.data(), payload.size());
}  // hanele_done_response()

}  // namespace detail

inline void handle_response(
    rpc_context& context,
    connection_manager_status cm_status,
    message_iterator& iter) {
  if (CMSTATUS_DONE == cm_status) {  // in most case
    detail::handle_done_response(context, iter);  // inlined
    return;
  }
  if (CMSTATUS_DEADLINE_EXCEEDED == cm_status) {
    context.handle_deadline_exceed();
    return;
  }
  CHECK(false) << "Unexpected status: " << cm_status;
}  // handle_response()

}  // namespace rpcz
#endif  // RPCZ_HANDLE_RESPONSE_HPP
