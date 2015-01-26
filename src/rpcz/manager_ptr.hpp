#ifndef RPCZ_CONNECTION_MANAGER_PTR_H
#define RPCZ_CONNECTION_MANAGER_PTR_H

#include <boost/shared_ptr.hpp>

namespace rpcz {
class manager;
typedef boost::shared_ptr<manager> manager_ptr;
}  // namespace rpcz

#endif  // RPCZ_CONNECTION_MANAGER_PTR_H
