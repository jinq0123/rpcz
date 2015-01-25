// Copyright 2011 Google Inc. All Rights Reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: nadavs@google.com <Nadav Samet>
//         Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_RPC_ERROR_HPP
#define RPCZ_RPC_ERROR_HPP

#include <stdexcept>
#include <string>

#include <rpcz/rpcz_api.hpp>
#include <rpcz/status_code.hpp>  // for status_code

namespace rpcz {

class RPCZ_API rpc_error : public std::runtime_error {
 public:
  rpc_error(status_code status,
            int application_error_code,
            const std::string& error_message);

  virtual ~rpc_error() throw();

  inline status_code get_status() const {
    return status_;
  }

  inline std::string get_error_message() const {
    return error_message_;
  }

  inline int get_application_error_code() const {
    return application_error_code_;
  }

 private:
  status_code status_;
  int application_error_code_;
  std::string error_message_;
};

}  // namespace
#endif
