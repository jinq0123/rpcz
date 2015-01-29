// Copyright 2011 Google Inc. All Rights Reserved.
// Copyright 2015 Jin Qing.
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

#ifndef RPCZ_REQUESTER_HPP
#define RPCZ_REQUESTER_HPP

#include <string>
#include <set>

#include <google/protobuf/stubs/common.h>

#include <rpcz/response_message_handler.hpp>  // for response_message_handler
#include <rpcz/requester_ptr.hpp>
#include <rpcz/rpcz_api.hpp>

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
}  // namespace protobuf
}  // namespace google

namespace rpcz {
class closure;
class dealer_connection;

class RPCZ_API requester {
 public:
  explicit requester(const dealer_connection& conn);
  ~requester();

 public:
  // DEL
  //virtual void call_method(const std::string& service_name,
  //                        const google::protobuf::MethodDescriptor* method,
  //                        const google::protobuf::Message& request,
  //                        google::protobuf::Message* response,
  //                        rpc_controller* rpc_controller,
  //                        closure* done) = 0;

  // only used in cpp? Other language use string request.
  void async_request(
      const std::string& service_name,
      const google::protobuf::MethodDescriptor* method,
      const google::protobuf::Message& request,
      const response_message_handler& msg_handler,
      long timeout_ms);

  void sync_request(
      const std::string& service_name,
      const google::protobuf::MethodDescriptor* method,
      const google::protobuf::Message& request,
      long timeout_ms,
      google::protobuf::Message* response  // out
      );

  // DO NOT USE: this method exists only for language bindings and may be
  // removed.
  // XXX change it...
  //virtual void call_method0(const std::string& service_name,
  //                         const std::string& method_name,
  //                         const std::string& request,
  //                         std::string* response,
  //                         rpc_controller* rpc_controller,
  //                         closure* done) = 0;

  static requester_ptr make_shared(const dealer_connection& conn);

  // Creates an requester to the given endpoint.
  static requester_ptr make_shared(const std::string& endpoint);

private:
  // Use ptr to hide dealer_connection.hpp
  boost::shared_ptr<dealer_connection> dealer_conn_;
};  // class requester

}  // namespace rpcz

#endif  // RPCZ_REQUESTER_HPP
