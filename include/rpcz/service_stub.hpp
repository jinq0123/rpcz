// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_STUB_HPP
#define RPCZ_SERVICE_STUB_HPP

#include <string>
#include <google/protobuf/stubs/common.h>  // for GOOGLE_DISALLOW_EVIL_CONSTRUCTORS()
#include <rpcz/rpc_channel_ptr.hpp>

namespace rpcz {

class requester;
class rpc_error;

class service_stub {
 public:
  inline service_stub(rpc_channel_ptr channel,
                      const std::string& service_name) :
      channel_(channel),
      service_name_(service_name),
      default_timeout_ms_(-1) {}
  inline virtual ~service_stub() {}

 public:
  inline rpc_channel_ptr channel() { return channel_; }
  inline void set_default_timeout_ms(long ms) { default_timeout_ms_ = ms; }

 protected:
  rpc_channel_ptr channel_;
  std::string service_name_;
  long default_timeout_ms_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(service_stub);
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_STUB_HPP
