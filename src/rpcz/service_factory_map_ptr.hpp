// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Service factory map pointer.

#ifndef RPCZ_SERVICE_FACTORY_MAP_PTR_HPP
#define RPCZ_SERVICE_FACTORY_MAP_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {
class service_factory_map;
typedef boost::shared_ptr<service_factory_map> service_factory_map_ptr;
}  // namespace rpcz
#endif  // RPCZ_SERVICE_FACTORY_MAP_PTR_HPP
