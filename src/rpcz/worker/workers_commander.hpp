// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_WORKERS_COMMANDER_HPP
#define RPCZ_WORKERS_COMMANDER_HPP

#include <boost/make_shared.hpp>
#include <boost/scoped_array.hpp>
#include <rpcz/worker/worker_cmd_queue_ptr.hpp>
#include <rpcz/worker/worker_cmd.hpp>

namespace rpcz {

// Workers' commander.
// Send commands to workers.
// Push commands to worker's commands queue.
// Thread-safe.
class workers_commander {
 public:
  explicit workers_commander(int workers);

 public:
     int get_workers() const { return workers_; }

 public:
  // Non-thread-safe.
  void set_cmd_queue(unsigned int worker_index,
      const worker_cmd_queue_ptr& cmd_queue);

 public:
  inline void run_closure(unsigned int worker_index, closure* clsr);
  inline void start_rpc(unsigned int worker_index, rpc_controller* ctrl);
  inline void handle_data(unsigned int worker_index, const connection_info& info);  // XXX
  void handle_timeout(unsigned int worker_index, uint64 event_id);
  void register_svc(unsigned int worker_index, const connection_info& info);  // XXX
  void quit_worker(unsigned int worker_index);

 private:
  bool is_worker_index_legal(unsigned int worker_index) const;

 private:
  const int workers_;  // number of workers, >= 1
  boost::scoped_array<worker_cmd_queue_ptr> worker_cmd_queues_;
};

inline void workers_commander::run_closure(
    unsigned int worker_index, closure* clsr) {
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_ptr cmd(new b2w::run_closure_cmd(clsr));  // shared_ptr
  worker_cmd_queues_[worker_index]->push(cmd);
}

inline void workers_commander::start_rpc(
    unsigned int worker_index, rpc_controller* ctrl) {
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_ptr cmd(new b2w::start_rpc_cmd(ctrl));
  worker_cmd_queues_[worker_index]->push(cmd);
}

inline void workers_commander::handle_data(
    unsigned int worker_index, const connection_info& info) {  // XXX
  BOOST_ASSERT(is_worker_index_legal(worker_index));
  worker_cmd_ptr cmd(new b2w::handle_data_cmd(info));
  worker_cmd_queues_[worker_index]->push(cmd);
}

}  // namespace rpcz
#endif  // RPCZ_WORKERS_COMMANDER_HPP
