// Copyright 2011 Google Inc. All Rights Reserved.
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

#ifndef RPCZ_CONNECTION_MANAGER_H
#define RPCZ_CONNECTION_MANAGER_H

#include <string>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>

#include <rpcz/common.hpp>
#include <rpcz/manager_ptr.hpp>
#include <rpcz/manager_status.hpp>
#include <rpcz/service_factory_map.hpp>

namespace zmq {
class context_t;
class message_t;
class socket_t;
}  // namespace zmq

namespace rpcz {
class client_connection;
class closure;
class connection;
class connection_thread_context;
class message_iterator;
class sync_event;

// A manager is a multi-threaded asynchronous system for communication
// over ZeroMQ sockets.
// A manager can connect to a remote server:
//      manager::get()->connect("tcp://localhost:5557");
// manager and connection are thread-safe.
// manager is singleton.
class manager : boost::noncopyable {
 private:
  manager();

 public:
  // Dynamic singleton. Auto destruct.
  static inline connection_manager_ptr get();
  static bool is_destroyed();  // for debug

 public:
  // Blocks the current thread until all connection managers have completed.
  ~manager();

  // connects all manager threads to the given endpoint. On success
  // this method returns a connection object that can be used from any thread
  // to communicate with this endpoint.
  connection connect(const std::string& endpoint);

  // binds a socket to the given endpoint. 
  void bind(const std::string& endpoint, const service_factory_map& factories);
  // Unbind socket of the given endpoint.
  void unbind(const std::string& endpoint);

  // Executes the closure on one of the worker threads.
  void add(closure* closure);

  // Blocks this thread until terminate() is called from another thread.
  void run();

  // Releases all the threads that are blocked inside run()
  void terminate();

 public:
  // Get thread specific frontend socket.
  inline zmq::socket_t& get_frontend_socket();

 private:
  zmq::socket_t& new_frontend_socket();

 private:
  static connection_manager_ptr get_new();  // used by get()

 private:
  typedef boost::weak_ptr<manager> weak_ptr;
  typedef boost::lock_guard<boost::mutex> lock_guard;
  static weak_ptr this_weak_ptr_;
  static boost::mutex this_weak_ptr_mutex_;

 private:
  scoped_ptr<zmq::context_t> own_context_;
  zmq::context_t* context_;  // Use own_context_ or external context

  boost::thread broker_thread_;
  boost::thread_group worker_threads_;
  boost::thread_specific_ptr<zmq::socket_t> socket_;
  std::string frontend_endpoint_;
  scoped_ptr<sync_event> is_terminating_;
};  // class manager

connection_manager_ptr manager::get() {
  connection_manager_ptr p = this_weak_ptr_.lock();
  if (p) return p;
  return get_new();
}

zmq::socket_t& manager::get_frontend_socket() {
  zmq::socket_t* socket = socket_.get();
  if (socket)
      return *socket;
  return new_frontend_socket();
}

}  // namespace rpcz

#endif
