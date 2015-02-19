// Licensed under the Apache License, Version 2.0 (the "License");
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker function. Runs in worker thread.

#ifndef RPCZ_WORKER_HPP
#define RPCZ_WORKER_HPP

#include <string>
#include <boost/unordered_map.hpp>

#include <rpcz/common.hpp>  // for uint64

namespace zmq {
class context_t; 
class socket_t;
}  // namespace zmq

namespace rpcz {

class message_iterator;
class rpc_controller;

class worker {
 public:
  worker(size_t worker_index,
      const std::string& frontend_endpoint,
      zmq::context_t& context);
  ~worker();

 public:
  void operator()();

 private:
  void start_rpc(message_iterator& iter);
  void handle_data(zmq::socket_t& socket);
  void handle_timeout(message_iterator& iter);

 private:
  const size_t worker_index_;
  const std::string frontend_endpoint_;
  zmq::context_t& context_;

 private:
  typedef boost::unordered_map<uint64/*event_id*/, rpc_controller*>
      remote_response_map;
  remote_response_map remote_response_map_;
};

}  // namespace rpcz

#endif  // RPCZ_WORKER_HPP
