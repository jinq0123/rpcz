// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_RESPONSE_MESSAGE_HANDLER_H
#define RPCZ_RESPONSE_MESSAGE_HANDLER_H

#include <boost/function.hpp>

namespace rpcz {

// Low-level handler for rpc response.
// Used by C++ and all language bindings.
// Input response message data and size.
// Return false if message is illegal.
typedef boost::function<bool (const void* data, size_t size)>
  response_message_handler;

}  // namespace rpcz
#endif  // RPCZ_RESPONSE_MESSAGE_HANDLER_H
