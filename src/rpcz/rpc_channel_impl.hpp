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

#ifndef RPCZ_RPC_CHANNEL_IMPL_H
#define RPCZ_RPC_CHANNEL_IMPL_H

#include "connection.hpp"
#include "connection_manager_status.hpp"
#include "rpcz/rpc_channel.hpp"

namespace rpcz {

class closure;

class rpc_channel_impl: public rpc_channel {
 public:
  rpc_channel_impl(connection connection);

  virtual ~rpc_channel_impl();

  virtual void call_method(
      const std::string& service_name,
      const google::protobuf::MethodDescriptor* method,
      const google::protobuf::Message& request,
      google::protobuf::Message* response,
      rpc_controller* rpc_controller,
      closure* done);

  // only used in cpp?
  virtual void async_call(
      const std::string& service_name,
      const google::protobuf::MethodDescriptor* method,
      const google::protobuf::Message& request,
      const response_message_handler& msg_handler,
      const error_handler& err_handler,
      long deadline_ms);

  virtual void call_method0(
      const std::string& service_name,
      const std::string& method_name,
      const std::string& request,
      std::string* response,
      rpc_controller* rpc_controller,
      closure* done);

 private:
  void call_method_full(
      const std::string& service_name,
      const std::string& method_name,
      const ::google::protobuf::Message* request_msg,
      const std::string& request,
      ::google::protobuf::Message* response_msg,
      std::string* response_str,
      rpc_controller* rpc_controller,
      closure* done);

 private:
  connection connection_;
};  // class rpc_channel_impl

} // namespace rpcz

#endif /* RPCZ_SIMPLE_RPC_CHANNEL_IMPL_H_ */
