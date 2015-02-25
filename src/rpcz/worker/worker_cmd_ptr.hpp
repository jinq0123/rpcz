// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command shared ptr.

#ifndef RPCZ_WORKER_CMD_PTR_HPP
#define RPCZ_WORKER_CMD_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {
namespace b2w {
struct worker_cmd;
}  // namespace b2w
typedef boost::shared_ptr<b2w::worker_cmd> worker_cmd_ptr;
typedef boost::shared_ptr<b2w::handle_data_cmd> handle_data_cmd_ptr;
}  // namespace rpcz

#endif  // RPCZ_WORKER_CMD_PTR_HPP
