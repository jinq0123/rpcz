// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Service factory map. Thread-safe.

#include <rpcz/service_factory_map.hpp>

#include <boost/foreach.hpp>

namespace rpcz {

void service_factory_map::insert(const std::string& name,
    const service_factory_ptr& factory) {
  BOOST_ASSERT(factory);
  lock lk(mu_);
  map_[name] = factory;
}

void service_factory_map::erase(const std::string& name) {
  lock lk(mu_);
  map_.erase(name);
}

void service_factory_map::for_each(callback& cb) {
  lock lk(mu_);
  BOOST_FOREACH(const name_to_factory::value_type& v, map_)
    cb(v.first, v.second);
}

}  // namespace rpcz
