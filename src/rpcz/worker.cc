// Copyright 2011 Google Inc. All Rights Reserved.
// Copyright 2015 Jin Qing.
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

#include <rpcz/worker.hpp>

#include <zmq.hpp>

#include <rpcz/callback.hpp>  // for closure
#include <rpcz/internal_commands.hpp>  // for kReady
#include <rpcz/logging.hpp>  // for CHECK_EQ()
#include <rpcz/request_handler.hpp>
#include <rpcz/rpc_controller.hpp>  // for rpc_controller
#include <rpcz/zmq_utils.hpp>  // for send_empty_message()

namespace rpcz {

worker::worker(const std::string& frontend_endpoint,
               zmq::context_t& context)
    : frontend_endpoint_(frontend_endpoint),
      context_(context) {
}

worker::~worker() {
}

void worker::operator()() {
  zmq::socket_t socket(context_, ZMQ_DEALER);
  socket.connect(frontend_endpoint_.c_str());
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kWorkerReady);
  bool should_continue = true;
  do {
    message_iterator iter(socket);
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    using namespace b2w;  // broker to worker command
    switch (command) {
      case kWorkerQuit:
        should_continue = false;
        break;
      case kRunClosure:
        interpret_message<closure*>(iter.next())->run();
        break;
      case kHandleRouterData: {
          // XXX
        //request_handler* handler =
        //    interpret_message<request_handler*>(iter.next());
        //assert(handler);
        //handler->handle_request(iter);
        }
        handle_router_data(iter);
        break;
      case kHandleDealerData: {
          // XXX
        //rpc_controller* ctrl =
        //    interpret_message<rpc_controller*>(iter.next());
        //BOOST_ASSERT(ctrl);
        //ctrl->handle_response(iter);
        //delete ctrl;
        }
        handle_dealer_data(iter);
        break;
      case kHandleTimeout:
        handle_timeout(iter);
        break;
      default:
        CHECK(false);
        break;
    }  // switch
  } while (should_continue);
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, c2b::kWorkerDone);
}

void worker::handle_router_data(message_iterator& iter) {
    // XXX
}

void worker::handle_dealer_data(message_iterator& iter) {
    // XXX
}

void worker::handle_timeout(message_iterator& iter) {
    // XXX
}

}  // namespace rpcz
