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

#ifndef RPCZ_CONNECTION_H
#define RPCZ_CONNECTION_H

#include <rpcz/common.hpp>
#include <rpcz/manager_ptr.hpp>

namespace rpcz {
class message_vector;
class rpc_controller;

// Represents a XXXconnection to a server. Thread-safe.
class dealer_connection {
 public:
  explicit dealer_connection(uint64 dealer_index);

  // Asynchronously sends a request over the dealer socket.
  // request: a vector of messages to be sent. Does not take ownership of the
  //          request. The vector has to live valid at least until the request
  //          completes. It can be safely de-allocated inside the provided
  //          closure or after remote_response->wait() returns.
  // ctrl: controller to run handler on one of the worker threads
  //       when a response arrives or timeout expires.
  void send_request(message_vector& request, rpc_controller* ctrl);

 private:
  manager_ptr manager_;
  uint64 dealer_index_;
};

}  // namespace rpcz

#endif  // RPCZ_CONNECTION_H
