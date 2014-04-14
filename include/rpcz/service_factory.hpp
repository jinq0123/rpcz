// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_FACTORY_HPP
#define RPCZ_SERVICE_FACTORY_HPP

#include "service_ptr.hpp"

namespace rpcz {

class service_factory {
 public:
  service_factory() {};
  virtual ~service_factory() {};

 public:
  virtual service_ptr make() = 0;
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_FACTORY_HPP
