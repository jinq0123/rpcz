// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "rpc_context.hpp"

#include "rpcz/application_error_code.hpp"  // for application_error
#include "rpcz/rpc_error.hpp"

namespace rpcz {

void rpc_context::handle_deadline_exceed() {
  handle_error(status::DEADLINE_EXCEEDED, 0, "");
}

void rpc_context::handler_invalid_message() {
  handle_application_error(application_error::INVALID_MESSAGE, "");
}

void rpc_context::handle_application_error(
  int application_error_code,
  const std::string & error_message)
{
  handle_error(status::APPLICATION_ERROR,
      application_error_code, error_message);
}

void rpc_context::handle_error(status_code status,
  int application_error_code,
  const std::string & error_message)
{
  if (err_handler_)
    err_handler_(rpc_error(status, application_error_code, error_message));
}

}  // namespace rpcz
