// Licensed under the Apache License, Version 2.0 (the "License");
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_WORKER_THREAD_FUN_HPP
#define RPCZ_WORKER_THREAD_FUN_HPP

#include <string>

namespace zmq {
class context_t; 
}  // namespace zmq

namespace rpcz {

void worker_thread_fun(zmq::context_t& context,
                       const std::string& endpoint);

}  // namespace rpcz

#endif  // RPCZ_WORKER_THREAD_FUN_HPP
