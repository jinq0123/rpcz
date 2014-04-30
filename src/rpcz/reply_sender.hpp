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

#ifndef RPCZ_REPLY_SENDER_H
#define RPCZ_REPLY_SENDER_H

#include <zmq.hpp>  // for message_t

#include "client_connection.hpp"
#include "logging.hpp"  // for CHECK()
#include "rpcz/invalid_message_error.hpp"
#include "rpcz/reply_context.hpp"
#include "rpcz/rpc_controller.hpp"  // for status::APPLICATION_ERROR
#include "zmq_utils.hpp"  // for string_to_message()

// TODO: Use requester/responser instead of client/server

namespace rpcz {

// Copyable.
class reply_sender {
 public:
  // It copies reply_context.
  reply_sender(const reply_context& reply_context)
      : reply_context_(reply_context) {
  }

  virtual void send(const google::protobuf::Message& response) {
    if (NULL == reply_context_.connection)
        return;
    rpc_response_header generic_rpc_response;
    int msg_size = response.ByteSize();
    scoped_ptr<zmq::message_t> payload(new zmq::message_t(msg_size));
    if (!response.SerializeToArray(payload->data(), msg_size)) {
      throw invalid_message_error("Invalid response message");
    }
    send_generic_response(generic_rpc_response,
                        payload.release());
  }

  virtual void send0(const std::string& response) {
    if (NULL == reply_context_.connection)
        return;
    rpc_response_header generic_rpc_response;
    send_generic_response(generic_rpc_response,
                        string_to_message(response));
  }

  virtual void send_error(int application_error,
                          const std::string& error_message="") {
    if (NULL == reply_context_.connection)
        return;
    rpc_response_header generic_rpc_response;
    zmq::message_t* payload = new zmq::message_t();
    generic_rpc_response.set_status(status::APPLICATION_ERROR);
    generic_rpc_response.set_application_error(application_error);
    if (!error_message.empty()) {
      generic_rpc_response.set_error(error_message);
    }
    send_generic_response(generic_rpc_response,
                        payload);
  }

 private:
  // Sends the response back to a function server_impl through the reply function.
  // Takes ownership of the provided payload message.
  void send_generic_response(const rpc_response_header& generic_rpc_response,
                           zmq::message_t* payload) {
    size_t msg_size = generic_rpc_response.ByteSize();
    zmq::message_t* zmq_response_message = new zmq::message_t(msg_size);
    CHECK(generic_rpc_response.SerializeToArray(
            zmq_response_message->data(),
            msg_size));

    message_vector v;
    v.push_back(zmq_response_message);
    v.push_back(payload);
    assert(reply_context_.connection);
    reply_context_.connection.connection->reply(
        reply_context.event_id_, &v);
  }

private:
  const reply_context reply_context_;  // context copy
};  // class server_channel_impl

}  // namespace rpcz
#endif  // RPCZ_REPLY_SENDER_H