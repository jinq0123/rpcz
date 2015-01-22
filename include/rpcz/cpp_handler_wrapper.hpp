// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_CPP_HANDLER_WRAPPER_HPP
#define RPCZ_CPP_HANDLER_WRAPPER_HPP

#include "rpcz/error_handler.hpp"

namespace rpcz {

// Wrap C++ response handler type to response_message_handler.
// Response should be subtype of protocol::Message.
// The 2 input handlers will be copied.
// Error handler is used when message is invalid.
template <typename Response>
struct cpp_handler_wrapper {
public:
  typedef boost::function<void (const Response&)> handler;

public:
  inline explicit cpp_handler_wrapper(const handler& hdl,
      const error_handler& err_hdl) :
      handler_(hdl),
      error_handler_(err_hdl) {
  }

public:
  inline void operator()(const void* data, size_t size);

private:
  handler handler_;
  error_handler error_handler_;  // to handle invalid message
};

void handle_invalid_message(error_handler& err_handler);

template <typename Response>
inline void cpp_handler_wrapper<Response>::operator()(
    const void* data, size_t size) {
  BOOST_ASSERT(data);
  if (handler_.empty())
    return;  // ignore message

  Response resp;
  if (resp.ParseFromArray(data, size)) {
    handler_(resp);
    return;
  }

  // invalid message
  handle_invalid_message(error_handler_);
}  // operator()()

}  // namespace rpcz

#endif  // RPCZ_CPP_HANDLER_WRAPPER_HPP
