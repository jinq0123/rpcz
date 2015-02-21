// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Read and write connection_info through zmq.

#ifndef RPCZ_CONNECTION_INFO_ZMQ_HPP
#define RPCZ_CONNECTION_INFO_ZMQ_HPP

#include <zmq.hpp>
#include <rpcz/connection_info.hpp>
#include <rpcz/zmq_utils.hpp>  // for send_uint64()

namespace rpcz {

inline void read_connection_info(
    message_iterator& iter,
    connection_info* info) {
  BOOST_ASSERT(info);
  BOOST_ASSERT(iter.has_more());
  char is_router(interpret_message<char>(iter.next()));
  BOOST_ASSERT(iter.has_more());
  info->is_router = is_router;
  info->index = interpret_message<uint64>(iter.next());
  if (!is_router) return;
  BOOST_ASSERT(iter.has_more());
  info->sender = message_to_string(iter.next());
}

inline void write_connection_info(
    zmq::socket_t* socket,
    const connection_info& info,
    int flags=0) {
  BOOST_ASSERT(socket);
  send_char(socket, info.is_router, ZMQ_SNDMORE);
  if (info.is_router) {
    send_uint64(socket, info.index, ZMQ_SNDMORE);
    send_string(socket, info.sender, flags);
    return;
  }
  send_uint64(socket, info.index, flags);
}

}  // namesapce rpcz
#endif  // RPCZ_CONNECTION_INFO_ZMQ_HPP
