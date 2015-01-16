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

#include <string>

#include <zmq.hpp>

#include "connection_manager_status.hpp"  // for connection_manager_status
#include "internal_commands.hpp"  // for kReady
#include "logging.hpp"  // for CHECK_EQ()
#include "request_handler.hpp"
#include "rpc_context.hpp"  // for rpc_context
#include "rpcz/callback.hpp"  // for closure
#include "zmq_utils.hpp"  // for send_empty_message()

namespace rpcz {

void worker_thread_fun(zmq::context_t& context,
                       const std::string& endpoint) {
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
      case kRunClosure:
        interpret_message<closure*>(iter.next())->run();
        break;
      case kHandleRequest: {
        request_handler * handler =
            interpret_message<request_handler*>(iter.next());
        assert(handler);
        handler->handle_request(iter);
        }
        break;
      case kHandleResponse: {
        rpc_context* ctx =
            interpret_message<rpc_context*>(iter.next());
        BOOST_ASSERT(ctx);
        connection_manager_status status = connection_manager_status(
            interpret_message<uint64>(iter.next()));

        extern void handle_response(
            rpc_context & context,
            connection_manager_status status,
            message_iterator& iter);
        handle_response(*ctx, status, iter);

        delete ctx;
      }
    }
  }
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kWorkerDone);
}

}  // namespace rpcz
