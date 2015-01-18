// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_RPC_CONTEXT_HPP
#define RPCZ_RPC_CONTEXT_HPP

#include <string>
#include <boost/noncopyable.hpp>

#include "rpcz/error_handler.hpp"
#include "rpcz/response_message_handler.hpp"
#include "rpcz/status_code.hpp"

namespace rpcz {

class rpc_context : boost::noncopyable {
 public:
  rpc_context(
    const response_message_handler& msg_handler,
    const error_handler& err_handler,
    long deadline_ms)
      : msg_handler_(msg_handler),
        err_handler_(err_handler),
        deadline_ms_(deadline_ms) {
  }

  ~rpc_context() {}

 public:
  inline void handle_response_message(const void* data, size_t size);
  void handle_deadline_exceed();
  void handle_application_error(
      int application_error_code,
      const std::string & error_message);

 public:
  long get_deadline_ms() const {
    return deadline_ms_;
  }

 private:
  void handle_error(status_code status,
      int application_error_code,
      const std::string & error_message);

 private:
  response_message_handler msg_handler_;
  error_handler err_handler_;
  long deadline_ms_;
};

inline void rpc_context::handle_response_message(
    const void* data, size_t size) {
  BOOST_ASSERT(data);
  if (msg_handler_) {
    msg_handler_(data, size);
  }
}  // handle_response_message

}  // namespace rpcz
#endif  // RPCZ_RPC_CONTEXT_HPP
