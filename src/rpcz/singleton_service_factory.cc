// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "singleton_service_factory.hpp"

#include <boost/make_shared.hpp>

#include "mono_state_service.hpp"

namespace rpcz {

singleton_service_factory::singleton_service_factory(service & svc)
	: service_(svc) {
}

service_ptr singleton_service_factory::make()
{
	return boost::make_shared<mono_state_service>(service_);
}

}  // namespace rpcz