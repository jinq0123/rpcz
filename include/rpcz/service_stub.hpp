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
  typedef boost::function<void (const rpc_error&)> error_handler;

 public:
  inline ::rpcz::rpc_channel* channel() { return channel_; }
  inline void set_default_deadline_ms(long ms) { default_deadline_ms_ = ms; }

 protected:
  ::rpcz::rpc_channel* channel_;
  ::std::string service_name_;
  bool owns_channel_;
  long default_deadline_ms_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(service_stub);
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_STUB_H
