// Licensed under the Apache License, Version 2.0 (the "License");
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_ERROR_HANDLER_H
#define RPCZ_ERROR_HANDLER_H

#include <boost/function.hpp>

namespace rpcz {

class rpc_error;
typedef boost::function<void (const rpc_error&)> error_handler;

}  // namespace rpcz
#endif  // RPCZ_ERROR_HANDLER_H
