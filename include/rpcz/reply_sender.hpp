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

#ifndef RPCZ_REPLY_SENDER_H
#define RPCZ_REPLY_SENDER_H

#include <string>

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace zmq {
class message_t;
}  // namespace zmq

namespace rpcz {

class rpc_response_header;
struct reply_context;

class reply_sender {
 public:
  reply_sender(const reply_context& reply_context)
      : reply_context_(reply_context) {}
  ~reply_sender() {}

 public:
  void send(const google::protobuf::Message& response) const;
  void send0(const std::string& response) const;
  void send_error(int application_error,
      const std::string& error_message="") const;

 private:
  // Sends the response back.
  // Takes ownership of the provided payload message.
  void send_generic_response(
      const rpc_response_header& generic_rpc_response,
      zmq::message_t* payload) const;

private:
  const reply_context & reply_context_;
};  // class reply_sender

}  // namespace rpcz
#endif  // RPCZ_REPLY_SENDER_H