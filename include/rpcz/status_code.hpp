// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_STATUS_CODE_HPP
#define RPCZ_STATUS_CODE_HPP
namespace rpcz {
typedef int status_code;
namespace status {
static const status_code APPLICATION_ERROR = 0;
static const status_code DEADLINE_EXCEEDED = 1;
}  // namespace status
}  // namespace rpcz
#endif  // RPCZ_STATUS_CODE_HPP
