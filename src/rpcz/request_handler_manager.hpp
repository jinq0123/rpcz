// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Manager of request handlers.

#ifndef RPCZ_REQUEST_HANDLER_MANAGER
#define RPCZ_REQUEST_HANDLER_MANAGER

#include <boost/unordered_map.hpp>
#include <rpcz/common.hpp>  // for uint64
#include <rpcz/request_handler_ptr.hpp>
#include <rpcz/service_factory_map.hpp>

namespace rpcz {

struct connection_info;

// Managers this thread's request_handlers.
// Used in worker thread.
// Non-thread-safe.
class request_handler_manager {
public:
  request_handler_manager(void);
  virtual ~request_handler_manager(void);

public:
  inline request_handler& get_handler(const connection_info& info);

private:
  request_handler& insert_new_handler(const connection_info& info);

private:
  // Each connection has its own request handler.
  typedef boost::unordered_map<connection_info, request_handler_ptr> handler_map;
  handler_map handler_map_;
};

request_handler& request_handler_manager::get_handler(
    const connection_info& info) {
  handler_map::const_iterator iter = handler_map_.find(info);
  if (iter != handler_map_.end()) {
    BOOST_ASSERT((*iter).second);
    return *(*iter).second;
  }
  return insert_new_handler(info);
}

}  // namespace rpcz
#endif  // RPCZ_REQUEST_HANDLER_MANAGER
