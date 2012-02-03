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

#ifndef RPCZ_CONNECTION_MANAGER_H
#define RPCZ_CONNECTION_MANAGER_H

#include <string>
#include <boost/thread.hpp>
#include "rpcz/event_manager.h"
#include "rpcz/macros.h"

namespace zmq {
class context_t;
class message_t;
class socket_t;
}  // namespace zmq

namespace rpcz {
class Closure;
class Connection;
class ConnectionThreadContext;
class MessageVector;
class RemoteResponse;

namespace internal {
struct ThreadContext;
}  // namespace internal

// A ConnectionManager is a multi-threaded asynchronous system for client-side
// communication over ZeroMQ sockets. Each thread in a connection manager holds
// a socket that is connected to each server we speak to.
// The purpose of the ConnectionManager is to enable all threads in a program
// to share a pool of connections in a lock-free manner.
//
// ConnectionManager cm();
// Connection* c = cm.Connect("tcp://localhost:5557");
// 
// Now, it is possible to send requests to this backend fron any thread:
// c->SendRequest(...);
//
// ConnectionManager and Connection are thread-safe.
class ConnectionManager {
 public:
  // Constructs an EventManager that uses the provided ZeroMQ context and
  // has nthreads worker threads. The ConnectionManager does not take ownership
  // of the given ZeroMQ context and event manager. The provided event_manager
  // is used for executing user-supplied closures. If the event_manager is NULL
  // then the closure parameter supplied to SendRequest must be NULL.
  ConnectionManager(zmq::context_t* context, EventManager* event_manager);

  virtual ~ConnectionManager();

  // Connects all ConnectionManager threads to the given endpoint. On success
  // this method returns a Connection object that can be used from any thread
  // to communicate with this endpoint. Returns NULL in error.
  virtual Connection* Connect(const std::string& endpoint);

 private:
  zmq::context_t* context_;
  // The external event manager is used for running user-supplied closures when
  // responses arrive (or exceed their deadline).
  // The internal event manager is used as a container for the worker threads of
  // this connection manager.
  EventManager* external_event_manager_;

  inline zmq::socket_t& GetFrontendSocket();

  boost::thread thread_;
  boost::thread_specific_ptr<zmq::socket_t> socket_;
  friend class ConnectionImpl;
  std::string frontend_endpoint_;
  DISALLOW_COPY_AND_ASSIGN(ConnectionManager);
};

// Installs a SIGINT and SIGTERM handlers that causes all RPCZ's event loops
// to cleanly quit.
void InstallSignalHandler();

// Represents a connection to a server. Thread-safe.
class Connection {
 public:
  // Asynchronously sends a request over the connection.
  // request: a vector of messages to be sent. Does not take ownership of the
  //          request. The vector has to live valid at least until the request
  //          completes. It can be safely de-allocated inside the provided
  //          closure or after remote_response->Wait() returns.
  // response: points to a RemoteResponse object that will receive the response.
  //           this object must live at least until when the closure has been
  //           ran (and may be deleted by the closure).
  // deadline_ms - milliseconds before giving up on this request. -1 means
  //               forever.
  // closure - a closure that will be ran by the EventManager when a response
  //           arrives. The closure gets called also if the request times out.
  //           Hence, it is necessary to check response->status. If no
  //           EventManager was provided to the constructor, this must be NULL.
  virtual void SendRequest(
      MessageVector* request,
      RemoteResponse* remote_response,
      int64 deadline_ms,
      Closure* closure) = 0;

  virtual ~Connection() {};

 protected:
  Connection() {};

 private:
  DISALLOW_COPY_AND_ASSIGN(Connection);
};
}  // namespace rpcz
#endif
