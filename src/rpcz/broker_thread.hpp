// Copyright 2011 Google Inc. All Rights Reserved.
// Copyright 2015 Jin Qing.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: nadavs@google.com <Nadav Samet>
//         Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_BROKER_THREAD_H
#define RPCZ_BROKER_THREAD_H

#include <map>
#include <vector>

#include <rpcz/connection_info_ptr.hpp>
#include <rpcz/reactor.hpp>
#include <rpcz/worker/workers_commander_ptr.hpp>

namespace rpcz {
class closure;
class message_iterator;
class rpc_controller;
class sync_event;
struct connection_info;

// Client and server use the same broker_thread.
class broker_thread {
 public:
  broker_thread(
      zmq::context_t& context,
      sync_event* ready_event,
      zmq::socket_t* frontend_socket,
      const workers_commander_ptr& wkrs_cmdr);

 public:
  static void run(zmq::context_t& context,
                  sync_event* ready_event,
                  zmq::socket_t* frontend_socket,
                  const workers_commander_ptr& wkrs_cmdr);

 private:
  void wait_for_workers_ready_reply();
  void handle_frontend_socket(zmq::socket_t* frontend_socket);
  // DEL XXXXX
  //inline void begin_worker_command(
  //    const connection_info& conn_info, char command);
  //inline void begin_worker_command(
  //    size_t worker_index, char command);
  void handle_connect_command(
      const std::string& sender,
      const std::string& endpoint);
  void handle_bind_command(
      const std::string& sender,
      message_iterator& iter);
  void handle_bind_command(
      const std::string& sender,
      const std::string& endpoint);
  void handle_unbind_command(
      const std::string& sender,
      const std::string& endpoint);
  void handle_quit_command(message_iterator& iter);
  void handle_worker_done_command(const std::string& sender);

 private:
  // Callback on reactor deleted socket.
  void handle_socket_deleted(const std::string sender);
  inline void handle_router_socket(uint64 router_index);
  inline void handle_dealer_socket(uint64 dealer_index);
  inline void broker_thread::handle_socket(
    const connection_info_ptr& info, message_iterator& iter);
  void handle_timeout(uint64 event_id, size_t worker_index);

 private:
  inline void add_closure(closure* closure);
  inline void send_request(message_iterator& iter);
  inline void start_rpc(const connection_info& info,
                        rpc_controller* ctrl);
  inline void send_reply(message_iterator& iter);

 private:
  void register_service(message_iterator& iter);

 private:
  bool is_dealer_index_legal(uint64 dealer_index) const;
  bool is_router_index_legal(uint64 router_index) const;
  bool is_connection_info_legal(const connection_info& info) const;

 private:
  inline size_t get_worker_index(const connection_info& info) const;

 private:
  inline void forward_to(
      const connection_info& conn_info,
      message_iterator& iter);

 private:
  zmq::context_t& context_;
  int workers_;  // number of workers, >= 0
  reactor reactor_;
  std::vector<zmq::socket_t*> dealer_sockets_;  // of client.
  std::vector<zmq::socket_t*> router_sockets_;  // of server.
  typedef std::map<std::string, zmq::socket_t*> endpoint_to_socket;
  endpoint_to_socket bind_map_;  // for unbind
  zmq::socket_t* frontend_socket_;
  workers_commander_ptr workers_commander_;
};

}  // namespace rpcz

#endif  // RPCZ_BROKER_THREAD_H
