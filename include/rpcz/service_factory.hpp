// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_FACTORY_H
#define RPCZ_SERVICE_FACTORY_H

namespace rpcz {

class service;

class service_factory {
 public:
  service_factory() {};
  virtual ~service_factory() {};

 public:
  virtual service * create() = 0;
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_FACTORY_H
