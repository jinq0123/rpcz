// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_STUB_H
#define RPCZ_SERVICE_STUB_H

#include <string>
#include <boost/function.hpp>
#include <google/protobuf/stubs/common.h>  // for GOOGLE_DISALLOW_EVIL_CONSTRUCTORS()

namespace rpcz {

class rpc_channel;
class rpc_error;

class service_stub {
 public:
  service_stub(::rpcz::rpc_channel* channel,
               const std::string& service_name,
               bool owns_channel) :
      channel_(channel),
      service_name_(service_name),
      owns_channel_(owns_channel),
      default_deadline_ms_(-1) {}
  ~service_stub() {}

 public:
  typedef boost::function<void (const ::google::protobuf::Message&)>
      response_message_handler;
  typedef boost::function<void (const rpc_error&)> error_handler;

 public:
  inline ::rpcz::rpc_channel* channel() { return channel_; }
  inline void set_default_deadline_ms(long ms) { default_deadline_ms_ = ms; }
  inline void set_default_error_handler(const error_handler & handler) {
    default_error_handler_ = handler;
  }

 protected:
  ::rpcz::rpc_channel* channel_;
  ::std::string service_name_;
  bool owns_channel_;
  long default_deadline_ms_;
  error_handler default_error_handler_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(service_stub);
};

// Wrap specific response handler type to message handler type.
// Response should be subtype of protocol::Message.
// The input handler will be copied.
template <typename Response>
struct handler_wrapper
{
public:
  typedef boost::function<void (const Response&)> handler;

public:
  handler_wrapper(const handler& hdl) : hdl_(hdl) {
    BOOST_ASSERT(hdl);
  }

public:
  void operator()(const ::google::protobuf::Message& msg) {
    BOOST_ASSERT(hdl_);
    hdl_(*::google::protobuf::down_cast<const Response*>(&msg));
  }

private:
  handler hdl_;
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_STUB_H
