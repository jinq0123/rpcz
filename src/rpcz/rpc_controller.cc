// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Rpc controller to call the response handler.

#include <rpcz/rpc_controller.hpp>

#include <rpcz/application_error_code.hpp>  // for error_code
#include <rpcz/rpc_error.hpp>

namespace rpcz {

void rpc_controller::handle_timeout() {
  handle_error(error_code::TIMEOUT_EXPIRED, "");
}

void rpc_controller::handle_error(
    int error_code, const std::string& error_str) {
  if (handler_.empty()) return;
  rpc_error e(error_code, error_str);
  handler_(&e, NULL, 0);
}

}  // namespace rpcz
