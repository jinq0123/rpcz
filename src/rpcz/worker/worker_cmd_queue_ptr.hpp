// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command queue shared ptr.

#ifndef RPCZ_WORKER_CMD_QUEUE_PTR_HPP
#define RPCZ_WORKER_CMD_QUEUE_PTR_HPP

#include <boost/shared_ptr.hpp>
#include <tbb/tbb.h>  // for concurrent_bounded_queue
#include <rcpz/worker/worker_cmd_ptr.hpp>

namespace rpcz {
typedef tbb::concurrent_bounded_queue<worker_cmd_ptr> worker_cmd_queue;
typedef boost::shared_ptr<worker_cmd_queue> worker_cmd_queue_ptr;
}  // namespace rpcz

#endif  // RPCZ_WORKER_CMD_QUEUE_PTR_HPP
