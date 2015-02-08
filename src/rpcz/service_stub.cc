// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/service_stub.hpp>

#include <rpcz/zmq_channel.hpp>

namespace rpcz {

service_stub::service_stub(const std::string& endpoint)
    : channel_(new zmq_channel(endpoint)),  // shared_ptr
      default_timeout_ms_(-1) {
  BOOST_ASSERT(channel_);
}

}  // namespace rpcz
