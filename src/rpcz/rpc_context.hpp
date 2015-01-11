// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_RPC_CONTEXT_HPP
#define RPCZ_RPC_CONTEXT_HPP

#include <string>
#include "rpcz/error_handler.hpp"
#include "rpcz/response_message_handler.hpp"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

class closure;
class rpc_controller;

struct rpc_context {
  response_message_handler msg_handler;
  error_handler err_handler;
  long deadline_ms;


  // XXX deprecated members
  rpc_controller* rpc_controller;
  ::google::protobuf::Message* response_msg;
  std::string* response_str;
  closure* user_closure;
};

}  // namespace rpcz
#endif  // RPCZ_RPC_CONTEXT_HPP
