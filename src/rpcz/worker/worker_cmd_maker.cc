// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command.

namespace rpcz {

namespace b2w {

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

struct param_quit {
};

worker_cmd_ptr make_handle_timeout_cmd(uint64 event_id) {
  worker_cmd_ptr p = boost::make_shared<worker_cmd>();
  p->cmd = kHandleTimeout;
  param_handle_timeout& param = p->handle_timeout;
  param.event_id = event_id;
  return p;
}

worker_cmd_ptr make_register_svc_cmd(const connection_info& info) {
  worker_cmd_ptr p = boost::make_shared<worker_cmd>();
  p->cmd = kRegisterSvc;
  param_register_svc& param = p->register_svc;
  param.info = info;
  // XXX other...
  return p;
}

worker_cmd_ptr make_quit_worker_cmd() {
  worker_cmd_ptr p = boost::make_shared<worker_cmd>();
  p->cmd = kQuitworker;
  return p;
}

}  // namespace b2w
}  // namespace rpcz
#endif  // RPCZ_WORKER_CMD_HPP
