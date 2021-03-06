// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command.

#ifndef RPCZ_WORKER_CMD_HPP
#define RPCZ_WORKER_CMD_HPP

#include <boost/assert.hpp>
#include <zmq.hpp>  // for message_t

#include <rpcz/common.hpp>  // for uint64
#include <rpcz/connection_info.hpp>  // for connection_info
#include <rpcz/connection_info_ptr.hpp>

namespace rpcz {
class closure;
class rpc_controller;
class iservice;

// Messages sent from the broker to a worker thread:
namespace b2w {

enum worker_cmd_enum {
  kIllegalCmd = 0,

  kRunClosure,  // Run a closure.
  kStartRpc,  // Start an rpc. Map event_id to rpc_controller.
  kHandleData,  // Handle router/dealer socket data.
  kHandleTimeout,  // Handle request timeout.
  kRegisterSvc,  // Register service.
  kQuitWorker,  // Ask the worker to quit.

  kMaxCmd
};

struct worker_cmd {
  const worker_cmd_enum cmd;

  worker_cmd(worker_cmd_enum c) : cmd(c) {
    BOOST_ASSERT(c > kIllegalCmd);
    BOOST_ASSERT(c < kMaxCmd);
  }
};

struct run_closure_cmd : public worker_cmd {
  closure* clsr;

  run_closure_cmd(closure* cl)
      : worker_cmd(kRunClosure),
      clsr(cl) {
    BOOST_ASSERT(cl);
  }
};

struct start_rpc_cmd : public worker_cmd {
  rpc_controller* ctrl;

  start_rpc_cmd(rpc_controller* controller)
      : worker_cmd(kStartRpc),
        ctrl(controller) {
    BOOST_ASSERT(ctrl);
  }
};

struct handle_data_cmd : public worker_cmd {
  connection_info_ptr info;
  zmq::message_t header;
  zmq::message_t payload;

  handle_data_cmd()
      : worker_cmd(kHandleData) {}
};

struct handle_timeout_cmd : public worker_cmd {
  uint64 event_id;

  handle_timeout_cmd(uint64 ev_id);
};

struct register_svc_cmd : public worker_cmd {
  connection_info_ptr info;
  std::string name;
  iservice* svc;

  register_svc_cmd(const connection_info_ptr& conn_info,
      const std::string& svc_name, iservice* service);
};

struct quit_worker_cmd : public worker_cmd {
  quit_worker_cmd();
};

}  // namespace b2w
}  // namespace rpcz
#endif  // RPCZ_WORKER_CMD_HPP
