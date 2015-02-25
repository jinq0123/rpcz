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
  BOOST_ASSERT(worker_index < (unsigned int)workers_);
  BOOST_ASSERT(cmd_queue);
  // Should set once only.
  BOOST_ASSERT(!worker_cmd_queues_[worker_index]);
  worker_cmd_queues_[worker_index] = cmd_queue;
}

void workers_commander::handle_timeout(
    unsigned int worker_index, uint64 event_id) {
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_ptr cmd(new b2w::handle_timeout_cmd(event_id));  // shared_ptr
  worker_cmd_queues_[worker_index]->push(cmd);
}

void workers_commander::register_svc(
    unsigned int worker_index,
    const connection_info_ptr& info,
    const std::string& name,
    iservice* svc) {
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  BOOST_ASSERT(svc);
  worker_cmd_ptr cmd(new b2w::register_svc_cmd(info, name, svc));  // shared_ptr
  worker_cmd_queues_[worker_index]->push(cmd);
}

void workers_commander::quit_worker(
    unsigned int worker_index) {
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_ptr cmd(new b2w::quit_worker_cmd);  // shared_ptr
  worker_cmd_queues_[worker_index]->push(cmd);
}

bool workers_commander::is_worker_index_legal(
    unsigned int worker_index) const {
  return worker_index < (unsigned int)workers_
      && worker_cmd_queues_[worker_index];
}

}  // namespace rpcz
