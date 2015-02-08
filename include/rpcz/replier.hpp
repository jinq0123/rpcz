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

#ifndef RPCZ_REPLIER_HPP
#define RPCZ_REPLIER_HPP

#include <string>
#include <rpcz/connection.hpp>  // for reply()
#include <rpcz/connection_ptr.hpp>
#include <rpcz/rpcz_api.hpp>

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

// replier is copyable.
// Each request has its own replier, and should reply once.
// replier can be used in callback by copy.  // XXX Need example.
// replier's copy operator is quick.
// XXX More comments...
class RPCZ_API replier {
 public:
  replier(const connection_ptr& conn, const std::string& event_id)
      : conn_(conn), event_id_(event_id) {
    BOOST_ASSERT(conn);
  }
  ~replier() {}

 public:
  // reply(protocol::Message) is only for cpp use.
  void reply(const google::protobuf::Message& response) const {
    conn_->reply(event_id_, response);
  }
  //void reply(const std::string& response) const {
  //  conn_->reply(event_id_, response);
  //}
  void reply_error(int error_code,
      const std::string& error_message="") const {
    conn_->reply_error(event_id_, error_code, error_message);
  }

private:
  const connection_ptr conn_;
  const std::string event_id_;
};  // class replier

}  // namespace rpcz
#endif  // RPCZ_REPLIER_HPP
