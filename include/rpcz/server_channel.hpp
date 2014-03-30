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

#ifndef RPCZ_SERVER_CHANNEL_H
#define RPCZ_SERVER_CHANNEL_H

#include <string>

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

class server_channel {
 public:
  virtual void send(const google::protobuf::Message& response) = 0;
  virtual void send_error(int application_error,
                         const std::string& error_message = "") = 0;
  virtual ~server_channel() {}

  // Hack to allow language bindings to do the serialization at their
  // end. Do not use directly.
  virtual void send0(const std::string& response) = 0;
};

}  // namespace rpcz

#endif  // RPCZ_SERVER_CHANNEL_H
