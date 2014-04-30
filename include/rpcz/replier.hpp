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

#ifndef RPCZ_REPLIER_H
#define RPCZ_REPLIER_H

#include <assert.h>
#include <string>

#include "server_channel.hpp"

namespace rpcz {

template <typename MessageType>
class replier {
 public:
  explicit replier(server_channel* channel) :
      channel_(channel), replied_(false) {
  }

  ~replier() { }

  void send(const MessageType& response) {
    assert(!replied_);
    replied_ = true;

    channel_->send(response);
    // DEL delete channel_;
  }

  void Error(int application_error, const std::string& error_message="") {
    assert(!replied_);
    replied_ = true;

    channel_->send_error(application_error, error_message);
    // DEL delete channel_;
  }

 private:
  server_channel* channel_;  // XXX: use copy, no delete
  bool replied_;
};

}  // namespace rpcz
#endif  // RPCZ_REPLIER_H
