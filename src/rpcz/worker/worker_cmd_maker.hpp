// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command.

#ifndef RPCZ_WORKER_CMD_HPP
#define RPCZ_WORKER_CMD_HPP

namespace rpcz {

// Messages sent from the broker to a worker thread:
namespace b2w {

enum worker_cmd_enum {
  kIllegalCmd = 0,

  kRunClosure,  // Run a closure.
  kStartRpc,  // Start an rpc. Map event_id to rpc_controller.
  kHandleData,  // Handle router/dealer socket data.
  kHandleTimeout,  // Handle request timeout.
  kRegisterSvc,  // Register service.
  kQuitworker,  // Ask the worker to quit.

  kMaxCmd
};

struct param_run_closure;
struct param_start_rpc;
struct param_handle_data;
struct param_handle_timeout;
struct param_register_svc;
struct param_quit_worker;

struct worker_cmd {
  worker_cmd_enum cmd;
  // cmd parameter
  union {
    param_run_closure    run_closure;
    param_start_rpc      start_rpc;
    param_handle_data    handle_data;
    param_handle_timeout handle_timeout;
    param_register_svc   register_svc;
    param_quit_worker    quit_worker;
  };
};

class closure;
struct param_run_closure {
  closure* clsr;
};

class rpc_controller;
struct param_start_rpc {
  rpc_controller* ctrl;
};

struct param_handle_data {
  connection_info info;
  // XXX data to receive
};

struct param_handle_timeout {
  uint64 event_id;
};

struct param_register_svc {
  connection_info info;
  // XXX other data...
};

struct param_quit_worker {
};

inline worker_cmd_ptr make_run_closure_cmd();
inline worker_cmd_ptr make_start_rpc_cmd();
inline worker_cmd_ptr make_handle_data_cmd();
worker_cmd_ptr make_handle_timeout_cmd();
worker_cmd_ptr make_register_svc_cmd();
worker_cmd_ptr make_quit_worker_cmd();

inline worker_cmd_ptr make_run_closure_cmd(closure* clsr) {
  worker_cmd_ptr p = boost::make_shared<worker_cmd>();
  p->cmd = kRunClosure;
  param_run_closure& param = p->run_closure;
  param.clsr = clsr;
  return p;
}

inline worker_cmd_ptr make_start_rpc_cmd(rpc_controller* ctrl) {
  worker_cmd_ptr p = boost::make_shared<worker_cmd>();
  p->cmd = kStartRpc;
  param_start_rpc& param = p->start_rpc;
  param.ctrl = ctrl;
  return p;
}

inline worker_cmd_ptr make_handle_data_cmd(const connection_info& info) {
  worker_cmd_ptr p = boost::make_shared<worker_cmd>();
  p->cmd = kHandleData;
  param_handle_data& param = p->handle_data;
  param.info = info;
  // XXX other...
  return p;
}

}  // namespace b2w
}  // namespace rpcz
#endif  // RPCZ_WORKER_CMD_HPP
