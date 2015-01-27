// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Rpc context used to call the response handler.

#ifndef RPCZ_RPC_CONTEXT_HPP
#define RPCZ_RPC_CONTEXT_HPP

#include <string>
#include <boost/noncopyable.hpp>

#include <rpcz/response_message_handler.hpp>

namespace zmq {
class message_t;
}  // namespace zmq

namespace rpcz {

class message_iterator;

class rpc_controller : boost::noncopyable {
 public:
  inline rpc_controller(
      const response_message_handler& handler,
      long timeout_ms)
      : handler_(handler),
        timeout_ms_(timeout_ms),
        timeout_expired_(false) {}
  inline ~rpc_controller() {}

 public:
  inline long get_timeout_ms() const { return timeout_ms_; }
  inline void set_timeout_expired() { timeout_expired_ = true; }

 public:
  // Not inlined to inline all other private handlers in .cc file.
  // And hide rpcz.pb.h.
  void handle_response(message_iterator& iter);

 private:
  inline void handle_response_message(const void* data, size_t size);
  inline void handle_done_response(message_iterator& iter);

 private:
  // Error handlers are not inlined.
  void handle_timeout_expired();
  void handle_error(int error_code,
      const std::string& error_str);

 private:
  response_message_handler handler_;
  long timeout_ms_;
  bool timeout_expired_;
};  // class rpc_controller

}  // namespace rpcz
#endif  // RPCZ_RPC_CONTEXT_HPP
