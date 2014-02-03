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

#include "connection_manager.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <map>
#include <ostream>
#include <sstream>
#include <stddef.h>
#include <string>
#include <utility>
#include <vector>

#ifdef WIN32
#include <process.h>  // for getpid()
#else
#include <unistd.h>  // for getpid()
#endif

#include <zmq.hpp>
#include <google/protobuf/stubs/common.h>

#include "application_options.hpp"
#include "client_connection.hpp"
#include "client_request_callback.hpp"
#include "connection.hpp"
#include "internal_commands.hpp"
#include "logging.hpp"
#include "reactor.hpp"
#include "remote_response_wrapper.hpp"
#include "rpcz/callback.hpp"
#include "zmq_utils.hpp"

namespace rpcz {

connection_manager::weak_ptr connection_manager::this_weak_ptr_;
boost::mutex connection_manager::this_weak_ptr_mutex_;

namespace {
const uint64 kLargePrime = (1ULL << 63) - 165;
const uint64 kGenerator = 2;

typedef uint64 event_id;

class event_id_generator : boost::noncopyable {
 public:
  event_id_generator() {
    state_ = (reinterpret_cast<uint64>(this) << 32) + getpid();
  }

  event_id get_next() {
    state_ = (state_ * kGenerator) % kLargePrime;
    return state_;
  }

 private:
  uint64 state_;
};  // class event_id_generator
}  // namespace

void worker_thread(connection_manager* connection_manager,
                  zmq::context_t & context, std::string endpoint) {
  zmq::socket_t socket(context, ZMQ_DEALER);
  socket.connect(endpoint.c_str());
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kReady);
  bool should_quit = false;
  while (!should_quit) {
    message_iterator iter(socket);
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    switch (command) {
      case kWorkerQuit:
        should_quit = true;
        break;
      case krunclosure:
        interpret_message<closure*>(iter.next())->run();
        break;
      case krunserver_function: {
        connection_manager::server_function sf =
            interpret_message<connection_manager::server_function>(iter.next());
        uint64 socket_id = interpret_message<uint64>(iter.next());
        std::string sender(message_to_string(iter.next()));
        if (iter.next().size() != 0) {
          break;
        }
        std::string event_id(message_to_string(iter.next()));
        sf(client_connection(connection_manager, socket_id, sender, event_id),
           iter);
        }
        break;
      case kInvokeclient_request_callback: {
        client_request_callback cb =
            interpret_message<client_request_callback>(
                iter.next());
        connection_manager_status status = connection_manager_status(
            interpret_message<uint64>(iter.next()));
        cb(status, iter);
      }
    }
  }
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kWorkerDone);
}

class connection_manager_thread {
 public:
  connection_manager_thread(
      zmq::context_t & context,
      int nthreads,
      sync_event* ready_event,
      zmq::socket_t* frontend_socket) : 
    context_(context),
    frontend_socket_(frontend_socket),
    current_worker_(0) {
      wait_for_workers_ready_reply(nthreads);
      ready_event->signal();
      reactor_.add_socket(
          frontend_socket, new_permanent_callback(
              this, &connection_manager_thread::handle_frontend_socket,
              frontend_socket));
    }

  void wait_for_workers_ready_reply(int nthreads) {
    for (int i = 0; i < nthreads; ++i) {
      message_iterator iter(*frontend_socket_);
      std::string sender = message_to_string(iter.next());
      assert(!sender.empty());  // zmq id
      CHECK_EQ(0, iter.next().size());
      char command(interpret_message<char>(iter.next()));
      CHECK_EQ(kReady, command) << "Got unexpected command " << (int)command;
      workers_.push_back(sender);
    }
  }

  static void run(zmq::context_t & context,
                  int nthreads,
                  sync_event* ready_event,
                  zmq::socket_t* frontend_socket) {
    connection_manager_thread cmt(context, nthreads,
                                ready_event,
                                frontend_socket);
    cmt.reactor_.loop();
  }

  void handle_frontend_socket(zmq::socket_t* frontend_socket) {
    message_iterator iter(*frontend_socket);
    std::string sender = message_to_string(iter.next());
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    switch (command) {
      case kQuit:
        // Ask the workers to quit. They'll in turn send kWorkerDone.
        for (size_t i = 0; i < workers_.size(); ++i) {
          send_string(frontend_socket_, workers_[i], ZMQ_SNDMORE);
          send_empty_message(frontend_socket_, ZMQ_SNDMORE);
          send_char(frontend_socket_, kWorkerQuit, 0);
        }
        break;
      case kConnect:
        handle_connect_command(sender, message_to_string(iter.next()));
        break;
      case kBind: {
        std::string endpoint(message_to_string(iter.next()));
        connection_manager::server_function sf(
            interpret_message<connection_manager::server_function>(
                iter.next()));
        handle_bind_command(sender, endpoint, sf);
        break;
      }
      case kRequest:
        send_request(iter);
        break;
      case kReply:
        send_reply(iter);
        break;
      case kReady:
        CHECK(false);
        break;
      case kWorkerDone:
        workers_.erase(std::remove(workers_.begin(), workers_.end(), sender));
        current_worker_ = 0;
        if (workers_.size() == 0) {
          // All workers are gone, time to quit.
          reactor_.set_should_quit();
        }
        break;
      case krunclosure:
        add_closure(interpret_message<closure*>(iter.next()));
        break;
    }
  }

  inline void begin_worker_command(char command) {
    send_string(frontend_socket_, workers_[current_worker_], ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_char(frontend_socket_, command, ZMQ_SNDMORE);
    ++current_worker_;
    if (current_worker_ == workers_.size()) {
      current_worker_ = 0;
    }
  }

  inline void add_closure(closure* closure) {
    begin_worker_command(krunclosure);
    send_pointer(frontend_socket_, closure, 0);
  }

  inline void handle_connect_command(const std::string& sender,
                                   const std::string& endpoint) {
    zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_DEALER);
    connections_.push_back(socket);
    int linger_ms = 0;
    socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
    socket->connect(endpoint.c_str());
    reactor_.add_socket(socket, new_permanent_callback(
            this, &connection_manager_thread::handle_client_socket,
            socket));

    send_string(frontend_socket_, sender, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_uint64(frontend_socket_, connections_.size() - 1, 0);
  }

  inline void handle_bind_command(
      const std::string& sender,
      const std::string& endpoint,
      connection_manager::server_function server_function) {
    zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_ROUTER);
    int linger_ms = 0;
    socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
    socket->bind(endpoint.c_str());
    uint64 socket_id = server_sockets_.size();
    server_sockets_.push_back(socket);
    reactor_.add_socket(socket, new_permanent_callback(
            this, &connection_manager_thread::handle_server_socket,
            socket_id, server_function));

    send_string(frontend_socket_, sender, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, 0);
  }

  void handle_server_socket(uint64 socket_id,
                          connection_manager::server_function server_function) {
    message_iterator iter(*server_sockets_[socket_id]);
    begin_worker_command(krunserver_function);
    send_object(frontend_socket_, server_function, ZMQ_SNDMORE);
    send_uint64(frontend_socket_, socket_id, ZMQ_SNDMORE);
    forward_messages(iter, *frontend_socket_);
  }

  inline void send_request(message_iterator& iter) {
    uint64 connection_id = interpret_message<uint64>(iter.next());
    remote_response_wrapper remote_response_wrapper =
        interpret_message<rpcz::remote_response_wrapper>(iter.next());
    event_id event_id = event_id_generator_.get_next();
    remote_response_map_[event_id] = remote_response_wrapper.callback;
    if (remote_response_wrapper.deadline_ms != -1) {
      reactor_.run_closure_at(
          remote_response_wrapper.start_time +
              remote_response_wrapper.deadline_ms,
          new_callback(this, &connection_manager_thread::handle_timeout, event_id));
    }
    zmq::socket_t*& socket = connections_[connection_id];
    send_string(socket, "", ZMQ_SNDMORE);
    send_uint64(socket, event_id, ZMQ_SNDMORE);
    forward_messages(iter, *socket);
  }

  void handle_client_socket(zmq::socket_t* socket) {
    message_iterator iter(*socket);
    if (iter.next().size() != 0) {
      return;
    }
    if (!iter.has_more()) {
      return;
    }
    event_id event_id(interpret_message<event_id>(iter.next()));
    remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
    if (response_iter == remote_response_map_.end()) {
      return;
    }
    client_request_callback& callback = response_iter->second;
    begin_worker_command(kInvokeclient_request_callback);
    send_object(frontend_socket_, callback, ZMQ_SNDMORE);
    send_uint64(frontend_socket_, CMSTATUS_DONE, ZMQ_SNDMORE);
    forward_messages(iter, *frontend_socket_);
    remote_response_map_.erase(response_iter);
  }

  void handle_timeout(event_id event_id) {
    remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
    if (response_iter == remote_response_map_.end()) {
      return;
    }
    client_request_callback& callback = response_iter->second;
    begin_worker_command(kInvokeclient_request_callback);
    send_object(frontend_socket_, callback, ZMQ_SNDMORE);
    send_uint64(frontend_socket_, CMSTATUS_DEADLINE_EXCEEDED, 0);
    remote_response_map_.erase(response_iter);
  }

  inline void send_reply(message_iterator& iter) {
    uint64 socket_id = interpret_message<uint64>(iter.next());
    zmq::socket_t* socket = server_sockets_[socket_id];
    forward_messages(iter, *socket);
  }

 private:
  typedef std::map<event_id, client_request_callback>
      remote_response_map;
  typedef std::map<uint64, event_id> deadline_map;
  remote_response_map remote_response_map_;
  deadline_map deadline_map_;
  event_id_generator event_id_generator_;
  reactor reactor_;
  std::vector<zmq::socket_t*> connections_;
  std::vector<zmq::socket_t*> server_sockets_;
  zmq::context_t & context_;
  zmq::socket_t* frontend_socket_;
  std::vector<std::string> workers_;
  int current_worker_;
};

connection_manager::connection_manager()
  : context_(NULL),
    frontend_endpoint_("inproc://" + boost::lexical_cast<std::string>(this)
        + ".rpcz.connection_manager.frontend"),
    already_quit_(false)
{
  DLOG(INFO) << "connection_manager() ";
  application_options options;
  context_ = options.get_zmq_context();
  if (NULL == context_)
  {
      int zmq_io_threads = options.get_zmq_io_threads();
      assert(zmq_io_threads > 0);
      own_context_.reset(new zmq::context_t(zmq_io_threads));
      context_ = own_context_.get();
  }
  assert(context_);
  zmq::socket_t* frontend_socket = new zmq::socket_t(*context_, ZMQ_ROUTER);
  int linger_ms = 0;
  frontend_socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  frontend_socket->bind(frontend_endpoint_.c_str());
  int nthreads = options.get_connection_manager_threads();
  assert(nthreads > 0);
  for (int i = 0; i < nthreads; ++i) {
    worker_threads_.add_thread(
        new boost::thread(&worker_thread, this, boost::ref(*context_), frontend_endpoint_));
  }
  sync_event event;
  broker_thread_ = boost::thread(&connection_manager_thread::run,
                                 boost::ref(*context_), nthreads, &event,
                                 frontend_socket);
  event.wait();
}

connection_manager_ptr connection_manager::get()
{
  connection_manager_ptr p = this_weak_ptr_.lock();
  if (p) return p;
  lock_guard lock(this_weak_ptr_mutex_);
  p = this_weak_ptr_.lock();
  if (p) return p;
  p.reset(new connection_manager);
  this_weak_ptr_ = p;
  return p;
}

long connection_manager::use_count()
{
    return this_weak_ptr_.use_count();
}

zmq::socket_t& connection_manager::get_frontend_socket() {
  zmq::socket_t* socket = socket_.get();
  if (socket == NULL) {
    socket = new zmq::socket_t(*context_, ZMQ_DEALER);
    int linger_ms = 0;
    socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
    socket->connect(frontend_endpoint_.c_str());
    socket_.reset(socket);
  }
  return *socket;
}

connection connection_manager::connect(const std::string& endpoint) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kConnect, ZMQ_SNDMORE);
  send_string(&socket, endpoint, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
  uint64 connection_id = interpret_message<uint64>(msg);
  return connection(this, connection_id);
}

void connection_manager::bind(const std::string& endpoint,
                             server_function function) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kBind, ZMQ_SNDMORE);
  send_string(&socket, endpoint, ZMQ_SNDMORE);
  send_object(&socket, function, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
  return;
}

void connection_manager::add(closure* closure) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, krunclosure, ZMQ_SNDMORE);
  send_pointer(&socket, closure, 0);
  return;
}
 
void connection_manager::run() {
  is_termating_.wait();
}

void connection_manager::terminate() {
  quit_and_join();
  is_termating_.signal();
}

connection_manager::~connection_manager() {
  DLOG(INFO) << "~connection_manager()";
  terminate();
}

void connection_manager::quit_and_join()
{
  if (already_quit_) return;
  already_quit_ = true;  // not thread-safe
  DLOG(INFO) << "quit_and_join()...";
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kQuit, 0);
  broker_thread_.join();
  worker_threads_.join_all();
  DLOG(INFO) << "quit_and_join() done.";
}

}  // namespace rpcz
