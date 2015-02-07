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
#include <boost/shared_ptr.hpp>

#include <rpcz/ichannel.hpp>
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

class router_connection;
class rpc_header;
struct responder_info;

// responder is copyable.
// Each request has its own responder, and should reply once.
// responder can be used in callback by copy.  // XXX Need example.
// responder's copy operator is quick, which only copies a shared_ptr.
// XXX More comments...
class RPCZ_API responder : public ichannel {
 public:
  responder(router_connection& conn, const std::string& event_id);
  ~responder();

 public:
  // respond(protocol::Message) is only for cpp use.
  virtual void respond(const google::protobuf::Message& response);
  // rirtual void respond(const std::string& response);
  virtual void respond_error(int error_code,
      const std::string& error_message="");

 public:
  virtual void request(
      const google::protobuf::MethodDescriptor& method,
      const google::protobuf::Message& request,
      const response_message_handler& msg_handler,
      long timeout_ms) {};

 private:
  // Sends rpc header and payload.
  // Takes ownership of the provided payload message.
  void respond(const rpc_header& rpc_hdr,
            zmq::message_t* payload) const;

private:
  boost::shared_ptr<responder_info> info_;
};  // class responder

}  // namespace rpcz
#endif  // RPCZ_RESPONDER_HPP
