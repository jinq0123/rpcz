// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Request handler.

#ifndef RPCZ_REQUEST_HANDLER_H
#define RPCZ_REQUEST_HANDLER_H

#include <map>
#include <rpcz/common.hpp>  // for uint64
// XXX #include <rpcz/connection_ptr.hpp>
#include <rpcz/connection.hpp>

namespace rpcz {

class iservice;

// Each connection has its request_hander.
// Run in worker thread.
// Non-thread-safe.
class request_handler {
 public:
  explicit request_handler(const connection_info& conn_info);
  ~request_handler();

 public:
  void handle_request();

 public:
  // register_service() will take the ownership of input service.
  void register_service(rpcz::iservice* svc, const std::string& name);

 private:
  void unregister_service(const std::string& name);

 private:
  typedef std::map<std::string, rpcz::iservice*> service_map;
  service_map service_map_;  // Owns service. Delete in destructor.

  connection_info conn_info_;
};

}  // namespace rpcz

#endif  // RPCZ_REQUEST_HANDLER_H
