// Client connection.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "client_connection.hpp"

#include <zmq.hpp>

#include "connection_manager.hpp"
#include "internal_commands.hpp"
#include "zmq_utils.hpp"

namespace rpcz {

client_connection::client_connection(uint64 server_socket_idx,
                                     const std::string& sender)
    : manager_(*connection_manager::get()),
      server_socket_idx_(server_socket_idx),
      sender_(sender) {
};

// TODO: rename client_connection to reply_broker

void client_connection::reply(const std::string& event_id, 
                              message_vector* v) const {
  // TODO: Use socket as member if used in the same thread.
  zmq::socket_t& socket = manager_.get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kReply, ZMQ_SNDMORE);
  send_uint64(&socket, server_socket_idx_, ZMQ_SNDMORE);
  send_string(&socket, sender_, ZMQ_SNDMORE);
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_string(&socket, event_id, ZMQ_SNDMORE);
  write_vector_to_socket(&socket, *v);
}

}  // namespace rpcz
