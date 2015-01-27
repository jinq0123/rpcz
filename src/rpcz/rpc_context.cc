// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/rpc_context.hpp>

#include <rpcz/application_error_code.hpp>  // for application_error
#include <rpcz/rpc_error.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_response_header
#include <rpcz/zmq_utils.hpp>  // for message_iterator

namespace rpcz {

// Run in worker thread.
void rpc_context::handle_response(
    message_iterator& iter) {
  if (!timeout_expired_) {
    // in most cases
    handle_done_response(iter);
    return;
  }
  handle_timeout_expired();
}  // handle_response()

inline void rpc_context::handle_response_message(
    const void* data, size_t size) {
  BOOST_ASSERT(data);
  if (handler_) {
    handler_(NULL, data, size);
  }
}  // handle_response_message

void rpc_context::handle_timeout_expired() {
  handle_error(status::DEADLINE_EXCEEDED, 0, "");
}

void rpc_context::handle_application_error(
  int application_error_code,
  const std::string& error_message) {
  handle_error(status::APPLICATION_ERROR,
      application_error_code, error_message);
}

void rpc_context::handle_error(status_code status,
                               int application_error_code,
                               const std::string& error_message) {
  if (handler_.empty()) return;
  rpc_error e(status, application_error_code, error_message);
  handler_(&e, NULL, 0);
}

// Must be implemented in .cc file because of rpc_response_header.
inline void rpc_context::handle_done_response(message_iterator& iter) {
  if (!iter.has_more()) {
    handle_application_error(application_error::INVALID_MESSAGE, "");
    return;
  }
  rpc_response_header generic_response;
  zmq::message_t& msg_in = iter.next();
  if (!generic_response.ParseFromArray(msg_in.data(), msg_in.size())) {
    handle_application_error(application_error::INVALID_MESSAGE, "");
    return;
  }
  if (generic_response.has_application_error()) {
    handle_application_error(
        generic_response.application_error(),
        generic_response.error());
    return;
  }
  if (!iter.has_more()) {
    handle_application_error(application_error::INVALID_MESSAGE, "");
    return;
  }

  // the most case
  const zmq::message_t& payload = iter.next();
  handle_response_message(payload.data(), payload.size());
}  // handle_done_response()

}  // namespace rpcz
