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
struct connection_info;

typedef boost::shared_ptr<connection_info> connection_info_ptr;

class RPCZ_API connection {
 public:
  connection(uint64 router_index, const std::string& sender);
  explicit connection(const std::string& endpoint);
  explicit connection(const connection_info_ptr& info);
  ~connection();

 public:
  void request(
      const google::protobuf::MethodDescriptor& method,
      const google::protobuf::Message& req,
      const response_message_handler& msg_handler,
      long timeout_ms) const;

 public:
     // XXXX use uint64 event_id
  void reply(const std::string& event_id,
      const google::protobuf::Message& resp) const;
  void reply_error(
      const std::string& event_id,
      int error_code,
      const std::string& error_message="") const;

 private:
  // Request over the connection.
  // date: a vector of messages to be sent.
  // ctrl: controller to run handler on one of the worker threads
  //       when a response arrives or timeout expires. Takes the
  //       ownership of ctrl.
  void request(message_vector& data, rpc_controller* ctrl) const;
  friend void request_connection(connection& conn,
      message_vector& data, rpc_controller* ctrl);  // for unit test

 private:
  // Sends rpc header and payload.
  // Takes ownership of the provided payload message.
  inline void reply(
      const rpc_header& rpc_hdr,
      zmq::message_t* payload) const;
  inline void reply(message_vector& data) const;

 private:
  void init(bool is_router, uint64 index, const std::string& sender="");
  void init_dealer(const std::string& endpoint);
  void init_router(uint64 router_index, const std::string& sender);

 private:
  boost::shared_ptr<manager> manager_;  // connection need worker threads
  connection_info_ptr info_;
};

}  // namespace rpcz
#endif  // RPCZ_CONNECTION_HPP
