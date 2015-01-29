// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Rpc channel shared pointer type.
#ifndef RPCZ_RPC_CHANNEL_PTR_HPP
#define RPCZ_RPC_CHANNEL_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {

class requester;
typedef boost::shared_ptr<requester> rpc_channel_ptr;

}  // namespace rpcz

#endif  // RPCZ_RPC_CHANNEL_H
