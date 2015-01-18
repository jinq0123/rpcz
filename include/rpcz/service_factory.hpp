// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_FACTORY_HPP
#define RPCZ_SERVICE_FACTORY_HPP

namespace rpcz {

class iservice;

class service_factory {
 public:
  service_factory() {};
  virtual ~service_factory() {};

 public:
  virtual iservice* create() = 0;
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_FACTORY_HPP
