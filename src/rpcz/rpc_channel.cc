#include "rpcz/rpc_channel.hpp"

#include <boost/make_shared.hpp>
#include "connection_manager.hpp"
#include "rpc_channel_impl.hpp"

namespace rpcz {

rpc_channel_ptr rpc_channel::make_shared(const connection& c) {
  return boost::make_shared<rpc_channel_impl>(c);
}

rpc_channel_ptr rpc_channel::make_shared(const std::string& endpoint) {
  return make_shared(connection_manager::get()->connect(endpoint));
}

}  // namespace rpcz
