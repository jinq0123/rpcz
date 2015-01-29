// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Requester shared pointer type.
#ifndef RPCZ_REQUESTER_PTR_HPP
#define RPCZ_REQUESTER_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {

class requester;
typedef boost::shared_ptr<requester> requester_ptr;

}  // namespace rpcz

#endif  // RPCZ_REQUESTER_PTR_HPP
