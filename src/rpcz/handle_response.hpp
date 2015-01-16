// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)
//
// Response handler. Run in worker thread.

#ifndef RPCZ_HANDLE_RESPONSE_HPP
#define RPCZ_HANDLE_RESPONSE_HPP

#include "connection_manager_status.hpp"  // for connection_manager_status

namespace rpcz {

class message_iterator;
class rpc_context;

void handle_response(
    rpc_context & context,
    connection_manager_status status,
    message_iterator& iter);

}  // namespace rpcz

#endif  // RPCZ_HANDLE_RESPONSE_HPP
