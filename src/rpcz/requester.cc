#include <rpcz/requester.hpp>

#include <boost/make_shared.hpp>
#include <rpcz/rpc_channel_impl.hpp>

namespace rpcz {

requester_ptr requester::make_shared(const dealer_connection& conn) {
  return boost::make_shared<rpc_channel_impl>(conn);
}

requester_ptr requester::make_shared(const std::string& endpoint) {
  return make_shared(dealer_connection(endpoint));
}

}  // namespace rpcz
