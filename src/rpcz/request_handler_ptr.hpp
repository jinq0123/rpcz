// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_REQUEST_HANDLER_PTR_H
#define RPCZ_REQUEST_HANDLER_PTR_H

#include <boost/shared_ptr.hpp>

namespace rpcz {

class request_handler;
typedef boost::shared_ptr<request_handler> request_handler_ptr;

}  // namespace rpcz

#endif  // RPCZ_REQUEST_HANDLER_PTR_H
