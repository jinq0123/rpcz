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

#ifndef RPCZ_APPLICATION_ERROR_CODE_HPP
#define RPCZ_APPLICATION_ERROR_CODE_HPP

#include <rpcz/rpcz.pb.h>  // for rpc_response_header

namespace rpcz {

typedef rpc_response_header::application_error_code application_error_code;

namespace application_error {
static const application_error_code APPLICATION_NO_ERROR = rpc_response_header::APPLICATION_NO_ERROR;
static const application_error_code INVALID_HEADER = rpc_response_header::INVALID_HEADER;
static const application_error_code NO_SUCH_SERVICE = rpc_response_header::NO_SUCH_SERVICE;
static const application_error_code NO_SUCH_METHOD = rpc_response_header::NO_SUCH_METHOD;
static const application_error_code INVALID_MESSAGE = rpc_response_header::INVALID_MESSAGE;
static const application_error_code METHOD_NOT_IMPLEMENTED = rpc_response_header::METHOD_NOT_IMPLEMENTED;
}  // namespace application_error

}  // namespace rpcz
#endif  // RPCZ_APPLICATION_ERROR_CODE_HPP
