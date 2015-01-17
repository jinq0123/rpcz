// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "rpc_context.hpp"

#include <boost/lexical_cast.hpp>
#include "rpcz/application_error_code.hpp"  // for application_error

namespace rpcz {

void rpc_context::set_failed(int application_error, const std::string& error_message) {
  set_status(status::APPLICATION_ERROR);
  error_message_ = error_message;
  application_error_code_ = application_error;
}

std::string rpc_context::to_string() const {
  std::string result =
      "status: " + rpc_response_header_status_code_Name(status_);
  if (status_ == status::APPLICATION_ERROR) {
    result += " (" + boost::lexical_cast<std::string>(
            application_error_code_) + ")";
  }
  if (!error_message_.empty()) {
    result += ": " + error_message_;
  }
  return result;
}

void rpc_context::set_handler_failed() {
  // Only for one reason.
  set_failed(application_error::INVALID_MESSAGE, "");
}

}  // namespace rpcz
