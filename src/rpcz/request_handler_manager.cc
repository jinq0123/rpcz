// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Manager of request handlers.
// XXXX rename it to request_handler_map
// Map connection info to request handler.

#include <rpcz/request_handler_manager.hpp>

#include <boost/foreach.hpp>

#include <rpcz/request_handler.hpp>
#include <rpcz/service_factory.hpp>

namespace rpcz {

request_handler_manager::request_handler_manager(void) {
}

request_handler_manager::~request_handler_manager(void) {
}

request_handler& request_handler_manager::insert_new_handler(
    const connection_info& info) {
  BOOST_ASSERT(handler_map_.find(info) == handler_map_.end());
  // New request_handler. TODO: delete request_handler on disconnection
  request_handler_ptr handler_ptr(new request_handler(info));  // shared_ptr
  handler_map_[info] = handler_ptr;
  return *handler_ptr;
}

}  // namespace rpcz
