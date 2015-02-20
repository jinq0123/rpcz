// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Manager of request handlers.

#include <rpcz/request_handler_manager.hpp>

#include <boost/foreach.hpp>

#include <rpcz/request_handler.hpp>
#include <rpcz/service_factory.hpp>

namespace rpcz {

request_handler_manager::request_handler_manager(void) {
}

request_handler_manager::~request_handler_manager(void) {
}

request_handler* request_handler_manager::insert_new_handler(
    const connection_info& info) {
  BOOST_ASSERT(handler_map_.find(info) == handler_map_.end());
  // New request_handler. TODO: delete request_handler on disconnection
  request_handler_ptr handler_ptr(new request_handler(info));  // shared_ptr
  // Create services for this handler...
  BOOST_FOREACH(const service_factory_map::value_type& v, factories) {
    iservice* svc = v.second->create();
    assert(svc);
    handler_ptr->register_service(svc, v.first);
  }
  handler_map_[info] = handler_ptr;
  // TODO: request_handler.set_client_connection(sender)
  return *handler_ptr;
}

}  // namespace rpcz
