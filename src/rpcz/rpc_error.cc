// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "rpcz/rpc_error.hpp"

#include <boost/lexical_cast.hpp>

namespace rpcz {

static std::string to_string(status_code status,
    int application_error_code,
    const std::string& error_message) {
  std::string result =
      "status: " + rpc_response_header_status_code_Name(status);
  if (status::APPLICATION_ERROR == status) {
    result += " (" + boost::lexical_cast<std::string>(
        application_error_code) + ")";
  }
  if (!error_message.empty()) {
    result += ": " + error_message;
  }
  return result;
}

rpc_error::rpc_error(status_code status,
                    int application_error_code,
                    const std::string& error_message) :
    std::runtime_error(to_string(status,
        application_error_code, error_message)),
    status_(status),
    application_error_code_(application_error_code),
    error_message_(error_message) {
}

rpc_error::~rpc_error() throw() {
}

}  // namespace rpcz
