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
#include <boost/shared_ptr.hpp>
#include <rpcz/rpcz_api.hpp>

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace zmq {
class message_t;
}  // namespace zmq

namespace rpcz {

class client_connection;
class rpc_response_header;
struct reply_context;

// replier is copyable.
// Each request has its own replier, and should reply once.
// replier can used in callback by copy.  // XXX Need example.
// replier's copy operator is quick, which only copies a shared_ptr.
// XXX More comments...
class RPCZ_API replier {
 public:
  replier(client_connection& connection, const std::string& event_id);
  ~replier();

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
  boost::shared_ptr<reply_context> reply_context_;
};  // class replier

}  // namespace rpcz
#endif  // RPCZ_REPLY_SENDER_H