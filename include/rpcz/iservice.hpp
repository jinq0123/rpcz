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

#ifndef RPCZ_ISERVICE_H
#define RPCZ_ISERVICE_H

#include <string>

#include <rpcz/replier.hpp>
#include <rpcz/rpcz_api.hpp>

namespace rpcz {

// Service interface.
class RPCZ_API iservice {
 public:
  // dispatch_request() is low-level request handler: requests are void*.
  // It is exposed for all language bindings.
  virtual void dispatch_request(const std::string& method,
                                const void* payload, size_t payload_len,
                                const responder& rspndr) = 0;
};

}  // namespace rpcz

#endif  // RPCZ_ISERVICE_H
