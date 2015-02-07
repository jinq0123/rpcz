// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_STUB_HPP
#define RPCZ_SERVICE_STUB_HPP

#include <boost/assert.hpp>
#include <google/protobuf/stubs/common.h>  // for GOOGLE_DISALLOW_EVIL_CONSTRUCTORS()
#include <rpcz/channel_ptr.hpp>
#include <rpcz/rpcz_api.hpp>

namespace rpcz {

class RPCZ_API service_stub {
 public:
  inline explicit service_stub(const channel_ptr& channel) :
      channel_(channel),  // copy
      default_timeout_ms_(-1) {
    BOOST_ASSERT(channel);
  }
  explicit service_stub(const std::string& endpoint);
  inline virtual ~service_stub() {}

 public:
  inline channel_ptr get_channel_ptr() const { return channel_; }
  inline void set_default_timeout_ms(long ms) { default_timeout_ms_ = ms; }

 protected:
  channel_ptr channel_;
  long default_timeout_ms_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(service_stub);
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_STUB_HPP
