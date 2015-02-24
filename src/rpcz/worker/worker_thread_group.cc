// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker thread group. Thread-safe.

#include <rpcz/worker/worker_thread_group.hpp>

#include <rpcz/worker/worker.hpp>  // for worker

namespace rpcz {

worker_thread_group::worker_thread_group(
    int threads,
    const std::string& frontend_endpoint,
    zmq::context_t& context)
    : threads_(threads),
      workers_(new scoped_worder[threads]),  // scoped_array
      workers_commander_(new workers_commander(threads)) {  // shared_ptr
  BOOST_ASSERT(threads > 0);
  for (int i = 0; i < threads; ++i) {
    workers_[i].reset(new worker(i, frontend_endpoint, context));
    thread_group_.add_thread(new boost::thread(
        boost::ref(*workers_[i])));
    // All commands are send through the worker's cmd queue.
    workers_commander_->set_cmd_queue(i, workers_[i]->get_cmd_queue());
  }
}

worker_thread_group::~worker_thread_group() {
  thread_group_.join_all();
}

void worker_thread_group::join_all() {  // blocking
  thread_group_.join_all();
}

}  // namespace rpcz
