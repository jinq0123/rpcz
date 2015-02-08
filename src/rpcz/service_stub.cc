// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/service_stub.hpp>

#include <rpcz/connection.hpp>

namespace rpcz {

service_stub::service_stub(const std::string& endpoint)
    : conn_(new connection(endpoint)),  // shared_ptr
      default_timeout_ms_(-1) {
  BOOST_ASSERT(conn_);
}

}  // namespace rpcz
