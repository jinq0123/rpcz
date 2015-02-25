// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker function. Runs in worker thread.

#ifndef RPCZ_WORKER_HPP
#define RPCZ_WORKER_HPP

#include <string>
#include <boost/unordered_map.hpp>

#include <rpcz/common.hpp>  // for uint64
#include <rpcz/request_handler_map.hpp>
#include <rpcz/worker/worker_cmd_ptr.hpp>
#include <rpcz/worker/worker_cmd_queue_ptr.hpp>

namespace zmq {
class context_t;
class message_t;
class socket_t;
}  // namespace zmq

namespace rpcz {

namespace b2w {
struct handle_data_cmd;
}  // namespace b2w

class message_iterator;
class rpc_controller;
class rpc_request_header;
class rpc_response_header;
struct connection_info;

class worker {
 public:
  worker(size_t worker_index,
      const std::string& frontend_endpoint,
      zmq::context_t& context);
  ~worker();

 public:
  void operator()();

 public:
  worker_cmd_queue_ptr get_cmd_queue() const;

 private:
  inline void run_closure(const worker_cmd_ptr& cmd);
  inline void start_rpc(const worker_cmd_ptr& cmd);
  inline void handle_data(const worker_cmd_ptr& cmd);
  void handle_timeout(const worker_cmd_ptr& cmd);
  void register_service(const worker_cmd_ptr& cmd);

 private:
  inline void handle_request(
      const rpc_request_header& req_hdr,
      const b2w::handle_data_cmd& cmd);
  inline void handle_response(
      const rpc_response_header& resp_hdr,
      const b2w::handle_data_cmd& cmd);
  inline void handle_done_resp(uint64 event_id, const zmq::message_t& response);
  void handle_error_resp(const rpc_response_header& resp_hdr);

 private:
  const size_t worker_index_;
  const std::string frontend_endpoint_;
  zmq::context_t& context_;

 private:
  worker_cmd_queue_ptr cmd_queue_;

 private:
  typedef boost::unordered_map<uint64/*event_id*/, rpc_controller*>
      remote_response_map;
  remote_response_map remote_response_map_;

  request_handler_map request_handler_map_;
};

}  // namespace rpcz

#endif  // RPCZ_WORKER_HPP
