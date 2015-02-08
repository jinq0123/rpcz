// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_CONNECTION_PTR_HPP
#define RPCZ_CONNECTION_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {
class connection;
typedef boost::shared_ptr<connection> connection_ptr;
}  // namespace rpcz

#endif  // RPCZ_CONNECTION_PTR_HPP
