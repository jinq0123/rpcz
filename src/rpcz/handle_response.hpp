// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
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
#include "rpcz/rpc_controller.hpp"  // for set_status()
#include "zmq_utils.hpp"  // for message_iterator

namespace rpcz {

namespace detail {

// Returns true if OK.
inline bool handle_done_response(
    rpc_context & context,
    message_iterator& iter) {
  if (!iter.has_more()) {
    context.set_failed(application_error::INVALID_MESSAGE, "");
    return false;
  }
  rpc_response_header generic_response;
  zmq::message_t& msg_in = iter.next();
  if (!generic_response.ParseFromArray(msg_in.data(), msg_in.size())) {
    context.set_failed(application_error::INVALID_MESSAGE, "");
    return false;
  }
  if (generic_response.status() != status::OK) {
    context.set_failed(generic_response.application_error(),
                       generic_response.error());
    return false;
  }

  if (!iter.has_more()) {
    context.set_failed(application_error::INVALID_MESSAGE, "");
    return false;
  }

  context.set_status(status::OK);  // XXX No need to set OK?
  zmq::message_t& payload = iter.next();
  response_message_handler & msg_handler = context.get_msg_handler();
  if (msg_handler.empty())
    return true;  // ignore response
  if (msg_handler(payload.data(), payload.size()))
    return true;
  context.set_failed(application_error::INVALID_MESSAGE, "");
    return false;
}  // hanele_done_response()

}  // namespace detail

inline void handle_response(
    rpc_context & context,
    connection_manager_status cm_status,
    message_iterator& iter) {
  if (CMSTATUS_DONE == cm_status) {  // in most case
    if (detail::handle_done_response(context, iter))  // inlined
      return;  // OK
  } else {
    CHECK(CMSTATUS_DEADLINE_EXCEEDED == cm_status)
        << "Unexpected status: " << cm_status;
    context.set_status(status::DEADLINE_EXCEEDED);
  }  // if-else

  BOOST_ASSERT(!context.ok());
  // XXX run error handler...

  // We call signal() before we execute closure since the closure may delete
  // the rpc_controller object (which contains the sync_event).
  // XXX Check sync_event is valid. signal() before closure has no use?
  // XXX context.rpc_controller->signal();
  //if (context.user_closure) {
  //  context.user_closure->run();
  //}
}  // handle_response()

}  // namespace rpcz
#endif  // RPCZ_HANDLE_RESPONSE_HPP
