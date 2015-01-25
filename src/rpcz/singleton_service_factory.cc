// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/singleton_service_factory.hpp>

#include <boost/make_shared.hpp>

#include <rpcz/mono_state_service.hpp>

namespace rpcz {

singleton_service_factory::singleton_service_factory(iservice& svc)
    : service_(svc) {
}

iservice* singleton_service_factory::create()
{
  return new mono_state_service(service_);
}

}  // namespace rpcz
