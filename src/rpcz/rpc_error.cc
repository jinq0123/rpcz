// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/rpc_error.hpp>

#include <boost/lexical_cast.hpp>

namespace rpcz {

static std::string to_string(
    int error_code, const std::string& error_str) {
  std::string result
      = "("
      + boost::lexical_cast<std::string>(error_code)
      + ")";
  if (!error_str.empty())
    result += ": " + error_str;
  return result;
}

rpc_error::rpc_error(int error_code, const std::string& error_str) :
    std::runtime_error(to_string(error_code, error_str)),
    error_code_(error_code),
    error_str_(error_str) {
}

rpc_error::~rpc_error() throw() {
}

}  // namespace rpcz
