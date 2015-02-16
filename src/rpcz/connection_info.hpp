// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Connection information structure.

#ifndef RPCZ_CONNECTION_INFO_HPP
#define RPCZ_CONNECTION_INFO_HPP

#include <string>
#include <rpcz/common.hpp>  // for uint64

namespace rpcz {
struct connection_info {
  bool is_router;  // ZMQ_ROUTER or ZMQ_DEALER
  uint64 index;  // router or dealer index
  std::string sender;  // Zmq sender id. Empty for dealer type.
};
}  // namesapce rpcz
#endif  // RPCZ_CONNECTION_INFO_HPP
