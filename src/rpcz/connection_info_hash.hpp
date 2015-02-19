// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Hash function of connection_info.

#ifndef RPCZ_CONNECTION_INFO_HASH_HPP
#define RPCZ_CONNECTION_INFO_HASH_HPP

#include <boost/functional/hash.hpp>
#include <rpcz/connection_info.hpp>

namespace rpcz {

inline std::size_t hash_value(const connection_info& info) {
  std::size_t seed = 0;
  using boost::hash_combine;
  hash_combine(seed, info.is_router);
  hash_combine(seed, info.index);
  hash_combine(seed, info.sender);
  return seed;
}

}  // namesapce rpcz
#endif  // RPCZ_CONNECTION_INFO_HASH_HPP
