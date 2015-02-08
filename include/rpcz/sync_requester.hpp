// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Synchronized rpc requester.
// Usage: sync_requester(connection_ptr).request(...);

#ifndef RPCZ_SYNC_REQUESTER_HPP
#define RPCZ_SYNC_REQUESTER_HPP

#include <rpcz/connection_ptr.hpp>
#include <rpcz/rpcz_api.hpp>

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

class RPCZ_API sync_requester {
 public:
  explicit sync_requester(const connection_ptr& conn);
  ~sync_requester();

 public:
  // Block until get response.
  void request(
      const google::protobuf::MethodDescriptor& method,
      const google::protobuf::Message& request,
      long timeout_ms,
      google::protobuf::Message* response  // out
      );

 private:
  connection_ptr conn_;
};  // class sync_requester

}  // namespace rpcz

#endif  // RPCZ_SYNC_REQUESTER_HPP
