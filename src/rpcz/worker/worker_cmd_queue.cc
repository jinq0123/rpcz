// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command queue. Thread-safe.

#include <rpcz/worker/worker_cmd_queue.hpp>

namespace rpcz {

void worker_cmd_queue::push(const worker_cmd_ptr& cmd) {
  cmds_.push(cmd);
  cond_.signal();
}

// block until get one
worker_cmd_ptr worker_cmd_queue::pop() {
  cond_.wait(lock);
  }

}

}  // namespace rpcz
