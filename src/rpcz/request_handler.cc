// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/request_handler.hpp>

#include <boost/foreach.hpp>

#include <rpcz/logging.hpp>
#include <rpcz/application_error_code.hpp>  // for error_code
#include <rpcz/iservice.hpp>  // for dispatch_request()
#include <rpcz/responder.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_request_header
#include <rpcz/zmq_utils.hpp>  // for message_iterator

namespace rpcz {

request_handler::request_handler(uint64 router_index,
                                 const std::string& sender)
    : router_conn_(router_index, sender) {
}

request_handler::~request_handler() {
  // Delete proto_rpc_service pointers.
  service_map map_copy = service_map_;
  BOOST_FOREACH(const service_map::value_type& v, map_copy)
      unregister_service(v.first);
  assert(service_map_.empty());
}

void request_handler::handle_request(message_iterator& iter) {
  if (!iter.has_more()) return;
  std::string event_id(message_to_string(iter.next()));  // TODO: uint64 event_id?
  if (!iter.has_more()) return;
  rpc_header rpc_hdr;
  replier replier_copy(router_conn_, event_id);
  zmq::message_t& msg = iter.next();
  if (!rpc_hdr.ParseFromArray(msg.data(), msg.size())) {
    // Handle bad rpc.
    DLOG(INFO) << "Received bad header.";
    replier_copy.send_error(error_code::INVALID_HEADER, "Invalid rpc_header.");
    return;
  }
  if (!iter.has_more()) return;
  zmq::message_t& payload = iter.next();
  if (iter.has_more()) return;
  const rpc_request_header& req_hdr = rpc_hdr.req_hdr();
  service_map::const_iterator service_it
      = service_map_.find(req_hdr.service());
  if (service_it == service_map_.end()) {
    // Handle invalid service.
    std::string error_str = "Invalid service: " + req_hdr.service();
    DLOG(INFO) << error_str;
    replier_copy.send_error(error_code::NO_SUCH_SERVICE, error_str);
    return;
  }
  rpcz::iservice* svc = service_it->second;
  assert(svc);
  svc->dispatch_request(req_hdr.method(),
                        payload.data(), payload.size(),
                        replier_copy);
}  // handle_request()

void request_handler::register_service(rpcz::iservice* svc,
                                       const std::string& name) {
  unregister_service(name);
  service_map_[name] = svc;
}

void request_handler::unregister_service(const std::string& name) {
  service_map::const_iterator iter = service_map_.find(name);
  if (iter == service_map_.end()) return;
  assert((*iter).second);
  delete (*iter).second;
  service_map_.erase(iter);
}

}  // namespace rpcz
