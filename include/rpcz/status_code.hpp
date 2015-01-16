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

#ifndef RPCZ_STATUS_CODE_HPP
#define RPCZ_STATUS_CODE_HPP

#include "rpcz/rpcz.pb.h"

namespace rpcz {

typedef rpc_response_header::status_code status_code;

namespace status {
static const status_code INACTIVE = rpc_response_header::INACTIVE;
static const status_code ACTIVE = rpc_response_header::ACTIVE;
static const status_code OK = rpc_response_header::OK;
static const status_code CANCELLED = rpc_response_header::CANCELLED;
static const status_code APPLICATION_ERROR = rpc_response_header::APPLICATION_ERROR;
static const status_code DEADLINE_EXCEEDED = rpc_response_header::DEADLINE_EXCEEDED;
static const status_code TERMINATED = rpc_response_header::TERMINATED;
}  // namespace status

}  // namespace rpcz
#endif  // RPCZ_STATUS_CODE_HPP
