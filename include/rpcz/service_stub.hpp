// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_STUB_H
#define RPCZ_SERVICE_STUB_H

#include <string>
#include <google/protobuf/stubs/common.h>  // for GOOGLE_DISALLOW_EVIL_CONSTRUCTORS()

namespace rpcz {

class rpc_channel;

class Service_Stub {
 public:
  Service_Stub(::rpcz::rpc_channel* channel,
               const std::string& service_name,
               bool owns_channel) :
      channel_(channel),
      service_name_(service_name),
      owns_channel_(owns_channel) {
  }
  ~Service_Stub() {
  }

  inline ::rpcz::rpc_channel* channel() { return channel_; }

 protected:
  ::rpcz::rpc_channel* channel_;
  ::std::string service_name_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Service_Stub);
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_STUB_H
