// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "sync_call_handler.hpp"

#include "rpcz/rpc_error.hpp"
#include "rpcz/application_error_code.hpp"

namespace rpcz {

void sync_call_handler::handle_error(const rpc_error& err) {
  error_ = err;
}

void sync_call_handler::handle_invalid_message() {
  handle_error(rpc_error(
      status::APPLICATION_ERROR,
      application_error::INVALID_MESSAGE,
      ""));
}

}  // namespace rpcz
