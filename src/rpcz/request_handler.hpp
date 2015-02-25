// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Request handler.

#ifndef RPCZ_REQUEST_HANDLER_H
#define RPCZ_REQUEST_HANDLER_H

#include <map>
#include <string>

#include <rpcz/service_factory_ptr.hpp>
#include <rpcz/connection.hpp>

namespace rpcz {

class iservice;
class rpc_request_header;
struct connection_info;

// Each connection will create a request_hander on first request.
// Run in worker thread.
// Worker has a map from connection_info to request_handler.
// Non-thread-safe.
class request_handler {
 public:
  explicit request_handler(const connection_info& conn_info);
  ~request_handler();

 public:
  void handle_request(const rpc_request_header& req_hdr,
                      const void* data, size_t len);

 public:
  // register_service() will take the ownership of input service.
  void register_service(const std::string& name, iservice* svc);

 private:
  void create_and_register_service(const std::string& name,
      const service_factory_ptr& factory);

 private:
  void unregister_service(const std::string& name);

 private:
  void create_services();

 private:
  typedef std::map<std::string, iservice*> service_map;
  service_map service_map_;  // Owns service. Delete in destructor.

  connection_info_ptr conn_info_;
};

}  // namespace rpcz

#endif  // RPCZ_REQUEST_HANDLER_H
