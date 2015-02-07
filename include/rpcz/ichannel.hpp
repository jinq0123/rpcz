// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Rpc channel interface to request and respond.

#ifndef RPCZ_ICHANNEL_HPP
#define RPCZ_ICHANNEL_HPP

#include <string>
#include <rpcz/response_message_handler.hpp>  // for response_message_handler
#include <rpcz/rpcz_api.hpp>  // for RPCZ_API

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

class RPCZ_API ichannel {
 public:
  // XXX only used in cpp? Other language use string request.
  virtual void request(
      const google::protobuf::MethodDescriptor& method,
      const google::protobuf::Message& request,
      const response_message_handler& msg_handler,
      long timeout_ms) = 0;

  // Extract a sync_requester helper... XXX
  //virtual void sync_request(
  //    const google::protobuf::MethodDescriptor& method,
  //    const google::protobuf::Message& request,
  //    long timeout_ms,
  //    google::protobuf::Message* response  // out
  //    ) = 0;

 public:
  // respond(protocol::Message) is only for cpp use.
  virtual void respond(const std::string& event_id,
      const google::protobuf::Message& response) = 0;
  // for language binding: virtual void respond(const std::string& response) = 0;
  virtual void respond_error(
      const std::string& event_id,
      int error_code,
      const std::string& error_message="") = 0;
};

}  // namespace rpcz

#endif  // RPCZ_ICHANNEL_HPP
