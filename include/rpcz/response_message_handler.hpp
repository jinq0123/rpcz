// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_RESPONSE_MESSAGE_HANDLER_H
#define RPCZ_RESPONSE_MESSAGE_HANDLER_H

#include <boost/function.hpp>

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace rpcz {

typedef boost::function<void (const ::google::protobuf::Message&)>
  response_message_handler;

}  // namespace rpcz
#endif  // RPCZ_RESPONSE_MESSAGE_HANDLER_H
