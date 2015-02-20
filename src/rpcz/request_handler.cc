// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/request_handler.hpp>

#include <boost/foreach.hpp>

#include <rpcz/application_error_code.hpp>  // for error_code
#include <rpcz/iservice.hpp>  // for dispatch_request()
#include <rpcz/logging.hpp>
#include <rpcz/replier.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_request_header
#include <rpcz/zmq_utils.hpp>  // for message_iterator
#include <rpcz/connection.hpp>
#include <rpcz/connection_info.hpp>

namespace rpcz {

request_handler::request_handler(const connection_info& conn_info) {
  conn_info_(conn_info);
}

request_handler::~request_handler() {
  // Delete proto_rpc_service pointers.
  service_map map_copy = service_map_;
  BOOST_FOREACH(const service_map::value_type& v, map_copy)
      unregister_service(v.first);
  assert(service_map_.empty());
}

void request_handler::handle_request() {
  connection_ptr conn(new connection(conn_info_));  // shared_ptr
  replier rep(conn, event_id);
  service_map::const_iterator service_it
      = service_map_.find(req_hdr.service());
  if (service_it == service_map_.end()) {
    // Handle invalid service.
    std::string error_str = "Invalid service: " + req_hdr.service();
    DLOG(INFO) << error_str;
    rep.reply_error(error_code::NO_SUCH_SERVICE, error_str);
    return;
  }
  rpcz::iservice* svc = service_it->second;
  assert(svc);
  svc->dispatch_request(req_hdr.method(),
                        payload.data(), payload.size(),
                        rep);
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
