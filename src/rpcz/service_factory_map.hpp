// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Service factory map. Thread-safe.

#ifndef RPCZ_SERVICE_FACTORY_MAP_HPP
#define RPCZ_SERVICE_FACTORY_MAP_HPP

#include <map>
#include <string>
#include <boost/function.hpp>
#include <boost/thread.hpp>  // for mutex
#include <rpcz/service_factory_ptr.hpp>

namespace rpcz {

class service_factory_map {
 public:
  void insert(const std::string& name, const service_factory_ptr& factory);
  void erase(const std::string& name);
  typedef boost::function<void (const std::string& name,
      const service_factory_ptr& factory)> callback;
  void for_each(callback& cb);

 private:
  typedef std::map<std::string, service_factory_ptr> name_to_factory;
  name_to_factory map_;
  boost::mutex mu_;
  typedef boost::unique_lock<boost::mutex> lock;
};  // class service_factory_map

}  // namespace rpcz
#endif  // RPCZ_SERVICE_FACTORY_MAP_HPP
