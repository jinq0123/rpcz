// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command shared ptr.

#ifndef RPCZ_WORKER_CMD_PTR_HPP
#define RPCZ_WORKER_CMD_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {
struct worker_cmd;
typedef boost::shared_ptr<worker_cmd> worker_cmd_ptr;
}  // namespace rpcz

#endif  // RPCZ_WORKER_CMD_PTR_HPP
