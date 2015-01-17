// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "rpcz/cpp_handler_wrapper.hpp"

#include "rpcz/application_error_code.hpp"
#include "rpcz/rpc_error.hpp"
#include "rpcz/status_code.hpp"

namespace rpcz {

void handle_invalid_message(error_handler& err_handler) {
  if (err_handler.empty())
    return;
  err_handler(rpc_error(
      status::APPLICATION_ERROR,
      application_error::INVALID_MESSAGE,
      ""));
}

}  // namespace rpcz
