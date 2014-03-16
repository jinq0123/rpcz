// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_FACTORY_PTR_H
#define RPCZ_SERVICE_FACTORY_PTR_H

#include <boost/shared_ptr.hpp>

namespace rpcz {

class service_factory;
typedef boost::shared_ptr<service_factory> service_factory_ptr;

}  // namespace rpcz

#endif  // RPCZ_SERVICE_FACTORY_PTR_H
