// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Zmq channel. Base class of dealer_channel and router_channel.

#ifndef RPCZ_ZMQ_CHANNEL_HPP
#define RPCZ_ZMQ_CHANNEL_HPP

#include <rpcz/ichannel.hpp>
#include "manager_ptr.hpp"  // XXX
#include <rpcz/common.hpp>

namespace zmq {
class message_t;
}  // namespace zmq

namespace rpcz {

class manager;
class message_vector;
class rpc_header;

class zmq_channel : public ichannel {
 public:
  zmq_channel(uint64 router_index, const std::string& sender);

 public:
  virtual void request(
      const google::protobuf::MethodDescriptor& method,
      const google::protobuf::Message& request,
      const response_message_handler& msg_handler,
      long timeout_ms);

 public:
  virtual void respond(const std::string& event_id,
      const google::protobuf::Message& response);
  virtual void respond_error(
      const std::string& event_id,
      int error_code,
      const std::string& error_message="");

 public:
  // XXX void reply(const std::string& event_id, message_vector* v) const;

 private:
  // Sends rpc header and payload.
  // Takes ownership of the provided payload message.
  void respond(const rpc_header& rpc_hdr,
               zmq::message_t* payload) const;

 private:
  manager_ptr manager_;
  const bool is_router_;  // ZMQ_ROUTER or ZMQ_DEALER
  const uint64 index_;  // router or dealer index
  const std::string sender_;  // Zmq sender id. Empty for dealer type.
};

}  // namespace rpcz
#endif  // RPCZ_CLIENT_CONNECTION_H
