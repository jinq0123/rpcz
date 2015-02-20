// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Request handler.
// Created on first request.

#include <rpcz/request_handler.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <rpcz/application_error_code.hpp>  // for error_code
#include <rpcz/connection.hpp>
#include <rpcz/connection_info.hpp>
#include <rpcz/iservice.hpp>  // for dispatch_request()
#include <rpcz/logging.hpp>
#include <rpcz/manager.hpp>
#include <rpcz/replier.hpp>
#include <rpcz/router_service_factories.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_request_header
#include <rpcz/service_factory.hpp>  // for create()
#include <rpcz/service_factory_map.hpp>  // for for_each()

namespace rpcz {

request_handler::request_handler(const connection_info& conn_info)
    : conn_info_(new connection_info(conn_info)) {  // shared_ptr
  create_services();
}

request_handler::~request_handler() {
  // Delete proto_rpc_service pointers.
  service_map map_copy = service_map_;
  BOOST_FOREACH(const service_map::value_type& v, map_copy)
      unregister_service(v.first);
  assert(service_map_.empty());
}

void request_handler::handle_request(
    const ::rpcz::rpc_request_header& req_hdr,
    const void* data, size_t len) {
  BOOST_ASSERT(data);
  connection_ptr conn(new connection(conn_info_));  // shared_ptr
  // TODO: One-way request: if req_hdr.has_event_id()
  replier rep(conn, req_hdr.event_id());
  service_map::const_iterator iter = service_map_.find(req_hdr.service());
  if (iter == service_map_.end()) {
    // Handle invalid service.
    std::string error_str = "Invalid service: " + req_hdr.service();
    DLOG(INFO) << error_str;
    rep.reply_error(error_code::NO_SUCH_SERVICE, error_str);
    return;
  }
  rpcz::iservice* svc = iter->second;
  assert(svc);
  svc->dispatch_request(req_hdr.method(), data, len, rep);
}  // handle_request()

void request_handler::register_service(
    const std::string& name, rpcz::iservice* svc) {
  BOOST_ASSERT(svc);
  unregister_service(name);
  service_map_[name] = svc;
}

void request_handler::create_and_register_service(
    const std::string& name,
    const service_factory_ptr& factory) {
  BOOST_ASSERT(factory);
  iservice* svc = factory->create();
  BOOST_ASSERT(svc);
  register_service(name, svc);
}

void request_handler::unregister_service(const std::string& name) {
  service_map::const_iterator iter = service_map_.find(name);
  if (iter == service_map_.end()) return;
  assert((*iter).second);
  delete (*iter).second;
  service_map_.erase(iter);
}

// Create services for this handler.
void request_handler::create_services() {
  if (conn_info_->is_router) {
    manager_ptr mgr = manager::get();
    BOOST_ASSERT(mgr);
    router_service_factories& factories = mgr->get_factories();
    service_factory_map_ptr router_factories
        = factories.get(conn_info_->index);
    if (!router_factories) return;
    router_factories->for_each(boost::bind(
        &request_handler::create_and_register_service, this, _1, _2));
  }
  // XXXX register service for dealer socket.
}

}  // namespace rpcz
