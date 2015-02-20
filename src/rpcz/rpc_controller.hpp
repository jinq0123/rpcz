// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Rpc controller to call the response handler.

#ifndef RPCZ_RPC_CONTEXT_HPP
#define RPCZ_RPC_CONTEXT_HPP

#include <string>
#include <boost/noncopyable.hpp>

#include <rpcz/response_message_handler.hpp>
#include <rpcz/common.hpp>  // for uint64

namespace zmq {
class message_t;
}  // namespace zmq

namespace rpcz {

class message_iterator;

class rpc_controller : boost::noncopyable {
 public:
  inline rpc_controller(uint64 event_id,
      const response_message_handler& handler,
      long timeout_ms)
      : event_id_(event_id),
        handler_(handler),
        timeout_ms_(timeout_ms),
        timeout_expired_(false) {}
  inline ~rpc_controller() {}

 public:
  inline uint64 get_event_id() const { return event_id_; }
  inline long get_timeout_ms() const { return timeout_ms_; }
  inline void set_timeout_expired() { timeout_expired_ = true; }  // DEL?

 public:
  inline void handle_response(const void* data, size_t size);

  // Error handlers are not inlined.

 private:
  void handle_timeout_expired();

 public:
  void handle_error(int error_code,
      const std::string& error_str);

 private:
  uint64 event_id_;
  response_message_handler handler_;
  long timeout_ms_;
  bool timeout_expired_;
};  // class rpc_controller

// Run in worker thread.
inline void rpc_controller::handle_response(
    const void* data, size_t size) {
  BOOST_ASSERT(data);
  if (handler_) {
    handler_(NULL, data, size);
  }
}  // handle_response_message

}  // namespace rpcz
#endif  // RPCZ_RPC_CONTEXT_HPP
