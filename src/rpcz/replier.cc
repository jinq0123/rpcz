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

#include <rpcz/replier.hpp>

#include <zmq.hpp>  // for message_t

#include <rpcz/client_connection.hpp>
#include <rpcz/invalid_message_error.hpp>
#include <rpcz/logging.hpp>  // for CHECK()
#include <rpcz/reply_context.hpp>
#include <rpcz/rpcz.pb.h>  // for rpc_response_header
#include <rpcz/status_code.hpp>  // for status
#include <rpcz/zmq_utils.hpp>  // for string_to_message()

// TODO: Use requester/responser instead of client/server

namespace rpcz {

// TODO: Do not use client_connection pointer, because connection may be deleted.
replier::replier(client_connection& connection,  // TODO: rename to reply_broker
                 const std::string& event_id)
    : reply_context_(new reply_context(&connection, event_id)) { // shared_ptr
};

replier::~replier() {
}

void replier::send(const google::protobuf::Message& response) const {
    assert(reply_context_->clt_connection);
    rpc_response_header generic_rpc_response;
    int msg_size = response.ByteSize();
    scoped_ptr<zmq::message_t> payload(new zmq::message_t(msg_size));
    if (!response.SerializeToArray(payload->data(), msg_size)) {
      throw invalid_message_error("Invalid response message");
    }
    send_generic_response(generic_rpc_response,
                          payload.release());
}

void replier::send0(const std::string& response) const {
    assert(reply_context_->clt_connection);
    rpc_response_header generic_rpc_response;
    send_generic_response(generic_rpc_response,
                          string_to_message(response));
}

void replier::send_error(int error_code,
        const std::string& error_message/* = "" */) const {
    assert(reply_context_->clt_connection);
    rpc_response_header generic_rpc_response;
    zmq::message_t* payload = new zmq::message_t();
    generic_rpc_response.set_error_code(error_code);
    if (!error_message.empty()) {
      generic_rpc_response.set_error_str(error_message);
    }
    send_generic_response(generic_rpc_response, payload);
}

// Sends the response back.
// Takes ownership of the provided payload message.
void replier::send_generic_response(
        const rpc_response_header& generic_rpc_response,
        zmq::message_t* payload) const {
    size_t msg_size = generic_rpc_response.ByteSize();
    zmq::message_t* zmq_response_message = new zmq::message_t(msg_size);
    CHECK(generic_rpc_response.SerializeToArray(
        zmq_response_message->data(), msg_size));

    message_vector v;
    v.push_back(zmq_response_message);
    v.push_back(payload);
    reply_context& rCtx = *reply_context_;
    assert(rCtx.clt_connection);
    assert(!rCtx.replied);  // Should reply only once.
    rCtx.clt_connection->reply(rCtx.event_id, &v);
    rCtx.replied = true;
}

}  // namespace rpcz
