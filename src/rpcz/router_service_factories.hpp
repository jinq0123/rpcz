// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Map zmq router socket to service factories. Thread-safe.
// One router can bind serveral service factories.

#ifndef RPCZ_ROUTER_SERVICE_FACTORY_MAP_HPP
#define RPCZ_ROUTER_SERVICE_FACTORY_MAP_HPP

#include <map>
#include <boost/thread.hpp>  // for mutex
#include <rpcz/common.hpp>  // for uint64
#include <rpcz/service_factory_map_ptr.hpp>

namespace rpcz {

class router_service_factories {
 public:
  void insert(uint64 router_index, const service_factory_map_ptr& factories);

 private:
  typedef std::map<uint64, service_factory_map_ptr> index_to_factories;
  index_to_factories map_;
  boost::mutex mu_;
  typedef boost::unique_lock<boost::mutex> lock;
};  // class router_service_factories

}  // namespace rpcz
#endif  // RPCZ_ROUTER_SERVICE_FACTORY_MAP_HPP
