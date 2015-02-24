// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Workers' commander shared ptr.

#ifndef RPCZ_WORKERS_COMMANDER_PTR_HPP
#define RPCZ_WORKERS_COMMANDER_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {
class workers_commander;
typedef boost::shared_ptr<workers_commander> workers_commander_ptr;
}  // namespace rpcz

#endif  // RPCZ_WORKERS_COMMANDER_PTR_HPP
