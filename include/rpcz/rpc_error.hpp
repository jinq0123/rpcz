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

namespace rpcz {

class RPCZ_API rpc_error : public std::runtime_error {
 public:
  rpc_error(int error_code, const std::string& error_str);
  virtual ~rpc_error() throw();

  inline std::string get_error_str() const {
    return error_str_;
  }

  inline int get_error_code() const {
    return error_code_;
  }

 private:
  int error_code_;
  std::string error_str_;
};

}  // namespace
#endif
