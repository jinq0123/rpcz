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

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

class closure;
class rpc_controller;

class rpc_context : boost::noncopyable {
 public:
  rpc_context(
    const response_message_handler& msg_handler,
    const error_handler& err_handler,
    long deadline_ms)
      : msg_handler_(msg_handler),
        err_handler_(err_handler),
        deadline_ms_(deadline_ms),
        status_(status::INACTIVE),
        application_error_code_(0) {
  }

  ~rpc_context() {}

 public:
  inline void handle_response_message(const void* data, size_t size);

  void set_status(status_code status) {
    status_ = status; 
  }
  long get_deadline_ms() const {
    return deadline_ms_;
  }

  void set_failed(int application_error_code, const std::string& message);

 private:
  std::string to_string() const;
  void set_handler_failed();

 private:
  response_message_handler msg_handler_;
  error_handler err_handler_;
  long deadline_ms_;

 private:
  status_code status_;
  std::string error_message_;
  int application_error_code_;

  // XXX deprecated members
public:
  rpc_controller* rpc_controller;
  // DEL ::google::protobuf::Message* response_msg;
  // DEL std::string* response_str;
  // XXX closure* user_closure;
};

inline void rpc_context::handle_response_message(
    const void* data, size_t size) {
  BOOST_ASSERT(data);
  if (msg_handler_) {
    if (msg_handler_(data, size))
      return;

    set_handler_failed();
  }
}  // handle_response_message

}  // namespace rpcz
#endif  // RPCZ_RPC_CONTEXT_HPP
