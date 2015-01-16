// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "rpc_context.hpp"

#include <boost/lexical_cast.hpp>

namespace rpcz {

void rpc_context::set_failed(int application_error, const std::string& error_message) {
  set_status(status::APPLICATION_ERROR);
  error_message_ = error_message;
  application_error_code_ = application_error;
}

std::string rpc_context::to_string() const {
  std::string result =
      "status: " + rpc_response_header_status_code_Name(get_status());
  if (get_status() == status::APPLICATION_ERROR) {
    result += "(" + boost::lexical_cast<std::string>(
            get_application_error_code())
           + ")";
  }
  std::string error_message = get_error_message();
  if (!error_message.empty()) {
    result += ": " + error_message;
  }
  return result;
}

}  // namespace rpcz
