// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_RESPONSE_MESSAGE_HANDLER_HPP
#define RPCZ_RESPONSE_MESSAGE_HANDLER_HPP

#include <boost/function.hpp>

namespace rpcz {

class rpc_error;

// Low-level handler for rpc response.
// Used by C++ and all language bindings.
// Input error pointer, response message data and size.
// If no error, then error == NULL, data != NULL.
typedef boost::function<void (const rpc_error* error,
    const void* data, size_t size)>
  response_message_handler;

}  // namespace rpcz
#endif  // RPCZ_RESPONSE_MESSAGE_HANDLER_HPP
