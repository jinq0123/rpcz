// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SINGLETON_SERVICE_FACTORY_HPP
#define RPCZ_SINGLETON_SERVICE_FACTORY_HPP

#include "rpcz/service_factory.hpp"

namespace rpcz {

class singleton_service_factory : public service_factory {
public:
    singleton_service_factory(service & svc);

public:
    virtual service * create();

private:
    service & service_;
};

}  // namespace rpcz
#endif  // RPCZ_SINGLETON_SERVICE_FACTORY_HPP
