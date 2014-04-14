// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_DEFAULT_SERVICE_FACTORY_HPP
#define RPCZ_DEFAULT_SERVICE_FACTORY_HPP

#include "service_factory.hpp"

namespace rpcz {

template <typename Service>
class default_service_factory : public service_factory {
 public:
  virtual service * create() {
	  return new Service;
  }
};

}  // namespace rpcz
#endif  // RPCZ_DEFAULT_SERVICE_FACTORY_HPP
