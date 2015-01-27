#include <rpcz/rpc_channel.hpp>

#include <boost/make_shared.hpp>
#include <rpcz/manager.hpp>
#include <rpcz/rpc_channel_impl.hpp>

namespace rpcz {

rpc_channel_ptr rpc_channel::make_shared(const dealer_connection& conn) {
  return boost::make_shared<rpc_channel_impl>(conn);
}

rpc_channel_ptr rpc_channel::make_shared(const std::string& endpoint) {
  return make_shared(manager::get()->connect(endpoint));
}

}  // namespace rpcz
