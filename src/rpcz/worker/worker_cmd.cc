// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker command.

#include <rpcz/worker/worker_cmd.hpp>

namespace rpcz {

// Messages sent from the broker to a worker thread:
namespace b2w {

handle_timeout_cmd::handle_timeout_cmd(uint64 ev_id)
    : worker_cmd(kHandleTimeout),
      event_id(ev_id) {
}

register_svc_cmd::register_svc_cmd(const connection_info& conn_info)
    : worker_cmd(kRegisterSvc),
      info(conn_info) {
  // XXX other data...
};

quit_worker_cmd::quit_worker_cmd()
    : worker_cmd(kQuitworker) {
};

}  // namespace b2w
}  // namespace rpcz
