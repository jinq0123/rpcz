// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_RPC_CONTEXT_HPP
#define RPCZ_RPC_CONTEXT_HPP

#include <string>
#include <boost/noncopyable.hpp>

#include <rpcz/response_message_handler.hpp>
#include <rpcz/status_code.hpp>

namespace zmq {
class message_t;
}  // namespace zmq

namespace rpcz {

class message_iterator;

class rpc_context : boost::noncopyable {
 public:
  inline rpc_context(
      const response_message_handler& handler,
      long deadline_ms)
      : handler_(handler),
        deadline_ms_(deadline_ms),
        deadline_exceeded_(false) {
  }

  inline ~rpc_context() {}

 public:
  inline long get_deadline_ms() const { return deadline_ms_; }
  inline void set_deadline_exceeded() { deadline_exceeded_ = true; }

 public:
  // Not inlined to inline all other private handlers in .cc file.
  // And hide rpcz.pb.h.
  void handle_response(message_iterator& iter);

 private:
  inline void handle_response_message(const void* data, size_t size);
  inline void handle_done_response(message_iterator& iter);
  inline const zmq::message_t* handle_response_header(message_iterator& iter);

 private:
  // Error handlers are not inlined.
  void handle_deadline_exceed();
  void handle_application_error(
      int application_error_code,
      const std::string& error_message);
  inline void handle_response_message(const void* data, size_t size);
  void handle_deadline_exceed();
  void handle_application_error(
      int application_error_code,
      const std::string& error_message);

 private:
  void handle_error(status_code status,
      int application_error_code,
      const std::string& error_message);
  inline void handle_done_response(
      message_iterator& iter);

 private:
  response_message_handler handler_;
  long deadline_ms_;
  bool deadline_exceeded_;
};

inline void rpc_context::handle_done_response(
    message_iterator& iter) {
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

  zmq::message_t& payload = iter.next();
  handle_response_message(payload.data(), payload.size());
}  // hanele_done_response()

}  // namespace rpcz
#endif  // RPCZ_RPC_CONTEXT_HPP
