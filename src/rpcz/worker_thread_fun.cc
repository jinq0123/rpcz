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

#include <rpcz/worker_thread_fun.hpp>

#include <zmq.hpp>

#include <rpcz/callback.hpp>  // for closure
#include <rpcz/internal_commands.hpp>  // for kReady
#include <rpcz/logging.hpp>  // for CHECK_EQ()
#include <rpcz/request_handler.hpp>
#include <rpcz/rpc_context.hpp>  // for rpc_context
#include <rpcz/zmq_utils.hpp>  // for send_empty_message()

namespace rpcz {

void worker_thread_fun(zmq::context_t& context,
                       const std::string& endpoint) {
  zmq::socket_t socket(context, ZMQ_DEALER);
  socket.connect(endpoint.c_str());
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kReady);
  bool should_continue = true;
  do {
    message_iterator iter(socket);
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    switch (command) {
      case kWorkerQuit:
        should_continue = false;
        break;
      case kRunClosure:
        interpret_message<closure*>(iter.next())->run();
        break;
      case kHandleRequest: {
        request_handler* handler =
            interpret_message<request_handler*>(iter.next());
        assert(handler);
        handler->handle_request(iter);
        }
        break;
      case kHandleResponse: {
        rpc_context* ctx =
            interpret_message<rpc_context*>(iter.next());
        BOOST_ASSERT(ctx);
        ctx->handle_response(iter);
        delete ctx;
        }
        break;
      default:
        CHECK(false);
        break;
    }  // switch
  } while (should_continue);
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kWorkerDone);
}

}  // namespace rpcz
