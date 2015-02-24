// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Make worker command shared ptr.

#ifndef RPCZ_WORKER_CMD_MAKER_HPP
#define RPCZ_WORKER_CMD_MAKER_HPP

#include <boost/make_shared.hpp>

#include <rpcz/worker/worker_cmd.hpp>
#include <rpcz/worker/worker_cmd_ptr.hpp>

namespace rpcz {

// Messages sent from the broker to a worker thread:
namespace b2w {

inline worker_cmd_ptr make_run_closure_cmd(closure* clsr);
inline worker_cmd_ptr make_start_rpc_cmd(rpc_controller* ctrl);
inline worker_cmd_ptr make_handle_data_cmd(rpc_controller* ctrl);
worker_cmd_ptr make_handle_timeout_cmd(uint64 event_id);
worker_cmd_ptr make_register_svc_cmd(const connection_info& info);
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
#endif  // RPCZ_WORKER_CMD_MAKER_HPP
