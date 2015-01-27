// Client connection.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/client_connection.hpp>

#include <zmq.hpp>

#include <rpcz/internal_commands.hpp>
#include <rpcz/manager.hpp>
#include <rpcz/zmq_utils.hpp>

namespace rpcz {

client_connection::client_connection(uint64 router_index,
                                     const std::string& sender)
    : manager_(*manager::get()),
      router_index_(router_index),
      sender_(sender) {
};

// TODO: rename client_connection to reply_broker

void client_connection::reply(const std::string& event_id, 
                              message_vector* v) const {
  zmq::socket_t& socket = manager_.get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kReply, ZMQ_SNDMORE);
  send_uint64(&socket, router_index_, ZMQ_SNDMORE);
  send_string(&socket, sender_, ZMQ_SNDMORE);
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_string(&socket, event_id, ZMQ_SNDMORE);
  write_vector_to_socket(&socket, *v);
}

}  // namespace rpcz
