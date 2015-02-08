// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Zmq connection. Zmq router or dealer.

#ifndef RPCZ_CONNECTION_HPP
#define RPCZ_CONNECTION_HPP

#include <boost/shared_ptr.hpp>

#include <rpcz/common.hpp>
#include <rpcz/rpcz_api.hpp>
#include <rpcz/response_message_handler.hpp>

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
}  // namespace protobuf
}  // namespace google

namespace zmq {
class message_t;
}  // namespace zmq

namespace rpcz {

class manager;
class message_vector;
class rpc_controller;
class rpc_header;

class RPCZ_API connection {
 public:
  connection(uint64 router_index, const std::string& sender);
  explicit connection(const std::string& endpoint);

 public:
  void request(
      const google::protobuf::MethodDescriptor& method,
      const google::protobuf::Message& req,
      const response_message_handler& msg_handler,
      long timeout_ms) const;

 public:
  void respond(const std::string& event_id,
      const google::protobuf::Message& resp) const;
  void respond_error(
      const std::string& event_id,
      int error_code,
      const std::string& error_message="") const;

 private:
  // Asynchronously sends a request over the connection.
  // date: a vector of messages to be sent. Does not take ownership of the
  //       data. The vector has to live valid at least until the request
  //       completes. It can be safely de-allocated inside the provided
  //       closure or after remote_response->wait() returns.
  // ctrl: controller to run handler on one of the worker threads
  //       when a response arrives or timeout expires.
  void request(message_vector& data, rpc_controller* ctrl) const;

 private:
  // Sends rpc header and payload.
  // Takes ownership of the provided payload message.
  void respond(const std::string& event_id,
      const rpc_header& rpc_hdr,
      zmq::message_t* payload) const;
  void respond(const std::string& event_id, message_vector* v) const;

 private:
  boost::shared_ptr<manager> manager_;
  const bool is_router_;  // ZMQ_ROUTER or ZMQ_DEALER
  const uint64 index_;  // router or dealer index
  const std::string sender_;  // Zmq sender id. Empty for dealer type.
};

}  // namespace rpcz
#endif  // RPCZ_CONNECTION_HPP
