// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Synchronized rpc requester.

#include <rpcz/sync_requester.hpp>

#include <rpcz/connection.hpp>
#include <rpcz/sync_call_handler.hpp>

namespace rpcz {

sync_requester::sync_requester(const connection_ptr& conn)
    : conn_(conn) {
  BOOST_ASSERT(conn);
}

sync_requester::~sync_requester() {
}

void sync_requester::request(
    const google::protobuf::MethodDescriptor& method,
    const google::protobuf::Message& request,
    long timeout_ms,
    google::protobuf::Message* response) {
  sync_call_handler handler(response);
  conn_->request(method, request,
      boost::ref(handler), timeout_ms);
  handler.wait();
  const rpc_error* err = handler.get_rpc_error();
  if (err)
    throw *err;
}

}  // namespace rpcz
