// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_FACTORY_HPP
#define RPCZ_SERVICE_FACTORY_HPP

#include <rpcz/rpcz_api.hpp>

namespace rpcz {

class iservice;

class RPCZ_API service_factory {
 public:
  service_factory() {};
  virtual ~service_factory() {};

 public:
  virtual iservice* create() = 0;
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_FACTORY_HPP
