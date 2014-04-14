// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_PTR_HPP
#define RPCZ_SERVICE_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {
class service;
typedef boost::shared_ptr<service> service_ptr;
}  // namespace rpcz

#endif  // RPCZ_SERVICE_PTR_HPP
