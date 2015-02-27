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

#ifndef RPCZ_CONNECTION_MANAGER_HPP
#define RPCZ_CONNECTION_MANAGER_HPP

#include <string>
#include <boost/atomic/atomic.hpp>  // for atomic_uint64_t
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <rpcz/common.hpp>
#include <rpcz/manager_ptr.hpp>
#include <rpcz/service_factory_map_ptr.hpp>

namespace zmq {
class context_t;
class socket_t;
}  // namespace zmq

namespace rpcz {

class closure;
class router_service_factories;
class sync_event;
class worker_thread_group;

// A manager is a multi-threaded asynchronous system for communication over ZeroMQ sockets.
// manager is thread-safe.
// manager is singleton.
class manager : boost::noncopyable {
 public:
  // Dynamic singleton. Auto destroyed.
  // See: (A dynamic) Singleton using weak_ptr and shared_ptr
  // http://boost.2283326.n4.nabble.com/A-dynamic-Singleton-using-weak-ptr-and-shared-ptr-td2581447.html
  // Constructed on first use, and destroyed if not referenced anymore.
  static inline manager_ptr get();
  static bool is_destroyed();  // for debug

 private:
  manager();
  // Blocks the current thread until all connections have completed.
  virtual ~manager();

  struct dyn_singleton_helper;
  struct dyn_singleton_deleter;
  friend dyn_singleton_deleter;

 public:
  // Binds a socket to the given endpoint.
  // And binds service factories to this socket.
  void bind(const std::string& endpoint,
      const service_factory_map_ptr& factories);
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
  inline uint64 get_next_event_id() { return next_event_id_.fetch_add(1); }
  router_service_factories& get_factories() { return *factories_; }

 private:
  zmq::socket_t& new_frontend_socket();
  void join_threads();

 private:
  scoped_ptr<zmq::context_t> context_;
  // thread specific frontend socket
  boost::thread_specific_ptr<zmq::socket_t> tss_fe_socket_;
  std::string frontend_endpoint_;

 private:
  boost::thread broker_thread_;
  scoped_ptr<worker_thread_group> worker_thread_group_;

 private:
  scoped_ptr<sync_event> terminated_;
  boost::atomic_uint64_t next_event_id_;

  // Map router index to factories.
  const scoped_ptr<router_service_factories> factories_;  // thread-safe
};  // class manager
}  // namespace rpcz

#include <rpcz/manager_dyn_singleton_helper.hpp>

namespace rpcz {

manager_ptr manager::get() {
  return dyn_singleton_helper::get_manager_ptr();
}

zmq::socket_t& manager::get_frontend_socket() {
  zmq::socket_t* socket = tss_fe_socket_.get();
  if (socket)
      return *socket;
  return new_frontend_socket();
}

}  // namespace rpcz
#endif  // RPCZ_CONNECTION_MANAGER_HPP
