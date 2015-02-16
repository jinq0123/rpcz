// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Shared pointer to connection information structure.

#ifndef RPCZ_CONNECTION_INFO_PTR_HPP
#define RPCZ_CONNECTION_INFO_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {
struct connection_info;
typedef boost::shared_ptr<connection_info> connection_info_ptr;
}  // namesapce rpcz
#endif  // RPCZ_CONNECTION_INFO_PTR_HPP
