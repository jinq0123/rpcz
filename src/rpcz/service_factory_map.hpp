#ifndef RPCZ_SERVICE_FACTORY_MAP_HPP
#define RPCZ_SERVICE_FACTORY_MAP_HPP

#include <map>
#include <string>
#include <rpcz/service_factory_ptr.hpp>

namespace rpcz {
typedef std::map<std::string, service_factory_ptr> service_factory_map;
}

#endif  // RPCZ_SERVICE_FACTORY_MAP_HPP
