// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_HANDLER_WRAPPER_HPP
#define RPCZ_HANDLER_WRAPPER_HPP

namespace rpcz {

// Wrap specific response handler type to response_message_handler.
// Response should be subtype of protocol::Message.
// The input handler will be copied.
template <typename Response>
struct handler_wrapper
{
public:
  typedef boost::function<void (const Response&)> handler;

public:
  explicit handler_wrapper(const handler& hdl) : hdl_(hdl) {
  }

public:
  void operator()(const ::google::protobuf::Message& msg) {
    if (hdl_)
      hdl_(*::google::protobuf::down_cast<const Response*>(&msg));
  }

private:
  handler hdl_;
};

}  // namespace rpcz

#endif  // RPCZ_HANDLER_WRAPPER_HPP
