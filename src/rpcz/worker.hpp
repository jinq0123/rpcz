// Licensed under the Apache License, Version 2.0 (the "License");
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker function. Runs in worker thread.

#ifndef RPCZ_WORKER_HPP
#define RPCZ_WORKER_HPP

#include <string>

namespace zmq {
class context_t; 
class socket_t;
}  // namespace zmq

namespace rpcz {

class message_iterator;

class worker {
 public:
  worker(size_t worker_index,
      const std::string& frontend_endpoint,
      zmq::context_t& context);
  ~worker();

 public:
  void operator()();

 private:
  void handle_data(zmq::socket_t& socket);
  void handle_timeout(message_iterator& iter);

 private:
  const size_t worker_index_;
  const std::string frontend_endpoint_;
  zmq::context_t& context_;
};

}  // namespace rpcz

#endif  // RPCZ_WORKER_HPP
