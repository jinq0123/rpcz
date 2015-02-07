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

#ifndef RPCZ_CLIENT_CONNECTION_H
#define RPCZ_CLIENT_CONNECTION_H

#include <rpcz/common.hpp>

namespace zmq {
class context_t;
}  // namespace zmq

namespace rpcz {

class manager;
class message_vector;

class router_connection {
 public:
  router_connection(uint64 router_index, const std::string& sender);

 public:
  void reply(const std::string& event_id, message_vector* v) const;

 private:
  manager& manager_;
  const uint64 router_index_;
  const std::string sender_;  // zmq sender id
};

}  // namespace rpcz

#endif  // RPCZ_CLIENT_CONNECTION_H
