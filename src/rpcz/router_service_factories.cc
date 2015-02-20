// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Map zmq router socket to service factories. Thread-safe.
// One router can bind serveral service factories.

#include <rpcz/router_service_factories.hpp>

namespace rpcz {

void router_service_factories::insert(uint64 router_index,
    const service_factory_map_ptr& factories) {
  BOOST_ASSERT(factories);
  lock lk(mu_);
  map_[router_index] = factories;
}

}  // namespace rpcz
#endif  // RPCZ_SERVICE_FACTORY_MAP_HPP
