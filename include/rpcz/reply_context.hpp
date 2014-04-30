// Context of reply. Used by replier.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_REPLY_CONTEXT_H
#define RPCZ_REPLY_CONTEXT_H

#include <string>

namespace rpcz {

class client_connection;

// Copyable.
struct reply_context {
    client_connection * client_connection;  // TODO: rename to reply_broker
    std::string event_id;
};

}  // namespace rpcz
#endif  // RPCZ_REPLY_CONTEXT_H
