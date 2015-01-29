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

#ifndef RPCZ_CPP_SERVICE_H
#define RPCZ_CPP_SERVICE_H

#include <rpcz/iservice.hpp>
#include <rpcz/rpcz_api.hpp>

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
class ServiceDescriptor;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

// Service for cpp.
class RPCZ_API cpp_service : public iservice {
 public:
  virtual const google::protobuf::ServiceDescriptor* GetDescriptor() = 0;

  virtual const google::protobuf::Message& GetRequestPrototype(
      const google::protobuf::MethodDescriptor*) const = 0;
  virtual const google::protobuf::Message& GetResponsePrototype(
      const google::protobuf::MethodDescriptor*) const = 0;

  // TODO: need request_context. Need client address in Ctr?
  // High-level handler.
  virtual void call_method(const google::protobuf::MethodDescriptor* method,
                           const google::protobuf::Message& request,
                           const responder& rspndr) = 0;

 public:
  // Low-level handler of iservice interface.
  virtual void dispatch_request(const std::string& method,
                                const void* payload, size_t payload_len,
                                const responder& rspndr);
};  // class cpp_service

}  // namespace rpcz
#endif  // RPCZ_CPP_SERVICE_H
