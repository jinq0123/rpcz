// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "request_handler.hpp"

#include <boost/foreach.hpp>

#include "rpcz/rpc_service.hpp"
#include "rpcz/rpcz.pb.h"  // for rpc_request_header
#include "server_channel_impl.hpp"
#include "zmq_utils.hpp"  // for message_iterator

namespace rpcz {

request_handler::request_handler() {
}

request_handler::~request_handler() {
  // Delete proto_rpc_service pointers.
  rpc_service_map map_copy = service_map_;
  BOOST_FOREACH(const rpc_service_map::value_type & v, map_copy)
      unregister_rpc_service(v.first);
  assert(service_map_.empty());
}

void request_handler::handle_request(const client_connection& connection,
                            message_iterator& iter) {
  if (!iter.has_more()) {
    return;
  }
  rpc_request_header rpc_request_header;
  scoped_ptr<server_channel> channel(new server_channel_impl(connection));
  {
    zmq::message_t& msg = iter.next();
    if (!rpc_request_header.ParseFromArray(msg.data(), msg.size())) {
      // Handle bad rpc.
      DLOG(INFO) << "Received bad header.";
      channel->send_error(application_error::INVALID_HEADER);
      return;
    };
  }
  if (!iter.has_more()) {
    return;
  }
  zmq::message_t& payload = iter.next();
  if (iter.has_more()) {
    return;
  }

  rpc_service_map::const_iterator service_it = service_map_.find(
      rpc_request_header.service());
  if (service_it == service_map_.end()) {
    // Handle invalid service.
    DLOG(INFO) << "Invalid service: " << rpc_request_header.service();
    channel->send_error(application_error::NO_SUCH_SERVICE);
    return;
  }
  rpcz::rpc_service* service = service_it->second;
  service->dispatch_request(rpc_request_header.method(),
                           payload.data(), payload.size(),
                           channel.release());
}  // handle_request()

void request_handler::register_rpc_service(rpcz::rpc_service* rpc_service,
                                           const std::string& name) {
  unregister_rpc_service(name);
  service_map_[name] = rpc_service;
}

void request_handler::unregister_rpc_service(const std::string& name) {
  rpc_service_map::const_iterator iter = service_map_.find(name);
  if (iter == service_map_.end()) return;
  assert((*iter).second);
  delete (*iter).second;
  service_map_.erase(iter);
}

}  // namespace rpcz
