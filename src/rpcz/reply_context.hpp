// Context of reply. Used by replier.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_REPLY_CONTEXT_H
#define RPCZ_REPLY_CONTEXT_H

#include <string>

namespace rpcz {

class router_connection;

struct reply_context {
  router_connection* router_conn;
  // TODO: Do not use router_connection pointer, because connection may be deleted.
  std::string event_id;
  bool replied;  // To assert one reply.

  inline reply_context(router_connection* conn, const std::string& evt_id)
      : router_conn(conn),
        event_id(evt_id),
        replied(false) {
    assert(conn);
  }
};

}  // namespace rpcz
#endif  // RPCZ_REPLY_CONTEXT_H
