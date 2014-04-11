// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_REQUEST_HANDLER_H
#define RPCZ_REQUEST_HANDLER_H

#include <map>

namespace rpcz {

class client_connection;
class message_iterator;
class rpc_service;

// Each client has its request_hander.
// Used in broker thread.
// TODO: Run in the same worker thread.
// Non-thread-safe.
class request_handler {
 public:
  request_handler();
  ~request_handler();

 public:
  void handle_request(const client_connection& connection,
                      message_iterator& iter);

 private:
  typedef std::map<std::string, rpcz::rpc_service*> rpc_service_map;
  rpc_service_map service_map_;  // Owns rpc_service. Delete in destructor.
};

}  // namespace rpcz

#endif  // RPCZ_REQUEST_HANDLER_H
