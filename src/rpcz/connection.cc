// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Zmq connection. Zmq router or dealer.

#include <rpcz/connection.hpp>

#include <zmq.hpp>

#include <rpcz/connection_info.hpp>
#include <rpcz/connection_info_zmq.hpp>  // for write_connection_info()
#include <rpcz/internal_commands.hpp>
#include <rpcz/invalid_message_error.hpp>
#include <rpcz/logging.hpp>  // for CHECK()
#include <rpcz/manager.hpp>
#include <rpcz/rpc_controller.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_header
#include <rpcz/zmq_utils.hpp>

// connection is thread-safe.

namespace rpcz {

// XXX block? exception?
static uint64 connect(const std::string& endpoint) {
  manager_ptr mgr = manager::get();
  BOOST_ASSERT(mgr);
  zmq::socket_t& socket = mgr->get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kConnect, ZMQ_SNDMORE);
  send_string(&socket, endpoint, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
  return interpret_message<uint64>(msg);
}

connection::connection(uint64 router_index,
                       const std::string& sender)
    : manager_(manager::get()) {
  BOOST_ASSERT(manager_);
  init_router(router_index, sender);
  BOOST_ASSERT(info_);
};

connection::connection(const std::string& endpoint)
    : manager_(manager::get()) {
  BOOST_ASSERT(manager_);
  init_dealer(endpoint);
  BOOST_ASSERT(info_);
}

connection::connection(const connection_info_ptr& info)
    : manager_(manager::get()),
      info_(info) {
  BOOST_ASSERT(info);
  BOOST_ASSERT(manager_);
}

connection::~connection() {
}

void connection::request(
    const google::protobuf::MethodDescriptor& method,
    const google::protobuf::Message& req,
    const response_message_handler& handler,
    long timeout_ms) const {
  rpc_header rpc_hdr;
  rpc_request_header* req_hdr = rpc_hdr.mutable_req_hdr();
  uint64 event_id = manager_->get_next_event_id();
  req_hdr->set_event_id(event_id);
  req_hdr->set_service(method.service()->name());
  req_hdr->set_method(method.name());

  size_t msg_size = rpc_hdr.ByteSize();
  scoped_ptr<zmq::message_t> msg_out(new zmq::message_t(msg_size));
  CHECK(rpc_hdr.SerializeToArray(msg_out->data(), msg_size));

  scoped_ptr<zmq::message_t> payload_out;
  size_t bytes = req.ByteSize();
  payload_out.reset(new zmq::message_t(bytes));
  if (!req.SerializeToArray(payload_out->data(), bytes)) {
    throw invalid_message_error("Request serialization failed.");
  }

  message_vector msg_vector;
  msg_vector.push_back(msg_out.release());
  msg_vector.push_back(payload_out.release());

  // rpc_controller will be deleted on response or timeout in worker thread.
  rpc_controller* ctrl = new rpc_controller(event_id, handler, timeout_ms);
  request(msg_vector, ctrl);
}

void connection::reply(uint64 event_id,
    const google::protobuf::Message& resp) const {
  int msg_size = resp.ByteSize();
  scoped_ptr<zmq::message_t> payload(new zmq::message_t(msg_size));
  if (!resp.SerializeToArray(payload->data(), msg_size)) {
    throw invalid_message_error("Invalid response message");
  }
  rpc_header rpc_hdr;
  rpc_response_header* resp_hdr = rpc_hdr.mutable_resp_hdr();
  BOOST_ASSERT(resp_hdr);
  resp_hdr->set_event_id(event_id);
  BOOST_ASSERT(rpc_hdr.has_resp_hdr());
  BOOST_ASSERT(!rpc_hdr.has_req_hdr());
  reply(rpc_hdr, payload.release());
}

// XXX for language binding
//void connection::reply(const std::string& event_id,
//    const std::string& response) {
//  rpc_header rpc_hdr;
//  (void)rpc_hdr.mutable_resp_hdr();
//  reply(rpc_hdr, string_to_message(response));
//}

void connection::reply_error(
    uint64 event_id, int error_code,
    const std::string& error_message/* = "" */) const {
  rpc_header rpc_hdr;
  rpc_response_header* resp_hdr = rpc_hdr.mutable_resp_hdr();
  BOOST_ASSERT(resp_hdr);
  zmq::message_t* payload = new zmq::message_t();
  resp_hdr->set_event_id(event_id);
  resp_hdr->set_error_code(error_code);
  if (!error_message.empty()) {
    resp_hdr->set_error_str(error_message);
  }
  reply(rpc_hdr, payload);
}

// register_service() will take the ownership of input service.
void connection::register_service(
    const std::string& name, iservice* svc) {
  BOOST_ASSERT(svc);
  zmq::socket_t* socket = &manager_->get_frontend_socket();
  send_empty_message(socket, ZMQ_SNDMORE);
  send_char(socket, c2b::kRegisterSvc, ZMQ_SNDMORE);
  write_connection_info(socket, *info_, ZMQ_SNDMORE);
  send_string(socket, name, ZMQ_SNDMORE);
  send_pointer(socket, svc);
}

void connection::request(
    message_vector& data,
    rpc_controller* ctrl) const {
  BOOST_ASSERT(ctrl);
  zmq::socket_t* socket = &manager_->get_frontend_socket();
  send_empty_message(socket, ZMQ_SNDMORE);
  send_char(socket, c2b::kRequest, ZMQ_SNDMORE);
  write_connection_info(socket, *info_, ZMQ_SNDMORE);
  send_pointer(socket, ctrl, ZMQ_SNDMORE);
  write_vector_to_socket(socket, data);
}

// Sends rpc header and payload.
// Takes ownership of the provided payload message.
inline void connection::reply(
    const rpc_header& rpc_hdr,
    zmq::message_t* payload) const {
  size_t msg_size = rpc_hdr.ByteSize();
  zmq::message_t* zmq_hdr_msg = new zmq::message_t(msg_size);
  CHECK(rpc_hdr.SerializeToArray(zmq_hdr_msg->data(), msg_size));

  message_vector v;
  v.push_back(zmq_hdr_msg);
  v.push_back(payload);
  reply(v);
}

inline void connection::reply(message_vector& data) const {
  zmq::socket_t* socket = &manager_->get_frontend_socket();
  send_empty_message(socket, ZMQ_SNDMORE);
  send_char(socket, c2b::kReply, ZMQ_SNDMORE);
  write_connection_info(socket, *info_, ZMQ_SNDMORE);
  write_vector_to_socket(socket, data);
}

void connection::init(bool is_router,
    uint64 index, const std::string& sender/*=""*/) {
  info_.reset(new connection_info);
  info_->is_router = is_router;
  info_->index = index;
  info_->sender = sender;
}

void connection::init_dealer(const std::string& endpoint) {
  uint64 dealer_index = connect(endpoint);
  init(false/*is_router*/, dealer_index);
}

void connection::init_router(uint64 router_index, const std::string& sender) {
  init(true/*is_router*/, router_index, sender);
}

}  // namespace rpcz
