// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Workers' commander. Thread-safe.

#include <rpcz/worker/workers_commander.hpp>

namespace rpcz {

workers_commander::workers_commander(int workers)
    : workers_(workers),
      worker_cmd_queues_(new worker_cmd_queue_ptr[workers]) {  // scoped_ptr
  BOOST_ASSERT(workers > 0);
}

// Non-thread-safe.
void workers_commander::set_cmd_queue(unsigned int worker_index,
    const worker_cmd_queue_ptr& cmd_queue) {
  BOOST_ASSERT(worker_index < workers_);
  BOOST_ASSERT(cmd_queue);
  // Should set once only.
  BOOST_ASSERT(!worker_cmd_queues_[worker_index]);
  worker_cmd_queues_[worker_index] = cmd_queue;
}

void workers_commander::handle_timeout(
    unsigned int worker_index, uint64 event_id) {
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_queues[worker_index] = b2w::make_handle_timeout_cmd(event_id);
}

void workers_commander::register_svc(
    unsigned int worker_index, const connection_info& info) {  // XXX
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_queues_[worker_index] = b2w::make_register_svc_cmd(info);
}

void workers_commander::quit_worker(
    unsigned int worker_index) {
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_queues_[worker_index] = b2w::make_quit_worker_cmd();
}

bool workers_commander::is_worker_index_legal(
    unsigned int worker_index) const {
  return worker_index < workers_
      && worker_cmd_queues_[worker_index];
}

}  // namespace rpcz
