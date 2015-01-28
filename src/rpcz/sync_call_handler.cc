// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/sync_call_handler.hpp>

#include <rpcz/error_code.hpp>
#include <rpcz/rpc_error.hpp>

namespace rpcz {

void sync_call_handler::handle_error(const rpc_error& err) {
  error_ = err;
}

void sync_call_handler::handle_invalid_message() {
  handle_error(rpc_error(
      error_code::INVALID_MESSAGE,
      ""));
}

}  // namespace rpcz
