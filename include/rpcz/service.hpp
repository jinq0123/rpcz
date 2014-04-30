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

#ifndef RPCZ_SERVICE_H
#define RPCZ_SERVICE_H

#include <assert.h>
#include <string>

#include "server_channel.hpp"

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
class ServiceDescriptor;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

template <typename MessageType>
class reply {
 public:
  explicit reply(server_channel* channel) :
      channel_(channel), replied_(false) {
  }

  ~reply() { }

  void send(const MessageType& response) {
    assert(!replied_);
    channel_->send(response);
    // DEL delete channel_;
    replied_ = true;
  }

  void Error(int application_error, const std::string& error_message="") {
    assert(!replied_);
    channel_->send_error(application_error, error_message);
    // DEL delete channel_;
    replied_ = true;
  }

 private:
  server_channel* channel_;  // XXX: use copy, no delete
  bool replied_;
};

class service {
 public:
  service() { };

  virtual ~service() {};

  virtual const google::protobuf::ServiceDescriptor* GetDescriptor() = 0;

  virtual const google::protobuf::Message& GetRequestPrototype(
      const google::protobuf::MethodDescriptor*) const = 0;
  virtual const google::protobuf::Message& GetResponsePrototype(
      const google::protobuf::MethodDescriptor*) const = 0;

  virtual void call_method(const google::protobuf::MethodDescriptor* method,
                          const google::protobuf::Message& request,
                          server_channel* server_channel) = 0;
};
}  // namespace
#endif
