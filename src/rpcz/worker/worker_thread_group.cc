// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker thread group. Thread-safe.

#include <rpcz/worker/worker_thread_group.hpp>

#include <rpcz/worker/worker.hpp>  // for worker
#include <rpcz/worker/workers_commander.hpp>

namespace rpcz {

worker_thread_group::worker_thread_group(int threads)
    : threads_(threads),
      workers_(new scoped_worker[threads]),  // scoped_array
      workers_commander_(new workers_commander(threads)) {  // shared_ptr
  BOOST_ASSERT(threads > 0);
  for (int i = 0; i < threads; ++i) {
    workers_[i].reset(new worker(i));
    thread_group_.add_thread(new boost::thread(boost::ref(*workers_[i])));
    // All commands are send through the worker's cmd queue.
    workers_commander_->set_cmd_queue(i, workers_[i]->get_cmd_queue());
  }
}

worker_thread_group::~worker_thread_group() {
  thread_group_.join_all();
}

workers_commander_ptr worker_thread_group::get_workers_commander() const {
  return workers_commander_;
}

void worker_thread_group::join_all() {  // blocking
  for (int i = 0; i < threads_; i++)
    workers_commander_->quit_worker(i);
  thread_group_.join_all();
}

}  // namespace rpcz
