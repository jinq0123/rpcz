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

#ifndef RPCZ_RESPONDER_HPP
#define RPCZ_RESPONDER_HPP

#include <string>
#include <rpcz/rpcz_api.hpp>
#include <rpcz/connection_ptr.hpp>
#include "connection.hpp"  // XXX

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

class rpc_header;

// responder is copyable.
// Each request has its own responder, and should reply once.
// responder can be used in callback by copy.  // XXX Need example.
// responder's copy operator is quick, which only copies a shared_ptr.
// XXX More comments...
class RPCZ_API responder {
 public:
  responder(const connection_ptr& channel, const std::string& event_id)
      : channel_(channel), event_id_(event_id) {
    BOOST_ASSERT(channel);
  }
  ~responder() {}

 public:
  // respond(protocol::Message) is only for cpp use.
  void respond(const google::protobuf::Message& response) const {
    channel_->respond(event_id_, response);
  }
  //void respond(const std::string& response) const {
  //  channel_->respond(event_id_, response);
  //}
  void respond_error(int error_code,
      const std::string& error_message="") const {
    channel_->respond_error(event_id_, error_code, error_message);
  }

private:
  const connection_ptr channel_;
  const std::string event_id_;
};  // class responder

}  // namespace rpcz
#endif  // RPCZ_RESPONDER_HPP
