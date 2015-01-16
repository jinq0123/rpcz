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
//
// Response handler. Run in worker thread.

#include "connection_manager_status.hpp"  // for connection_manager_status
#include "logging.hpp"  // for CHECK()
#include "rpc_context.hpp"  // for rpc_context
#include "rpcz/application_error_code.hpp"  // for application_error
#include "rpcz/callback.hpp"  // for run()
#include "rpcz/rpc_controller.hpp"  // for set_status()
#include "zmq_utils.hpp"  // for message_iterator

// XXX DEL extern handle_response(), use .h file.

namespace rpcz {

void handle_response(
    rpc_context & context,
    connection_manager_status status,
    message_iterator& iter) {
  switch (status) {
    case CMSTATUS_DEADLINE_EXCEEDED:
      context.set_status(
          status::DEADLINE_EXCEEDED);
      break;
    case CMSTATUS_DONE: {
        if (!iter.has_more()) {
          context.set_failed(
              application_error::INVALID_MESSAGE, "");
          break;
        }
        rpc_response_header generic_response;
        zmq::message_t& msg_in = iter.next();
        if (!generic_response.ParseFromArray(msg_in.data(), msg_in.size())) {
          context.set_failed(
              application_error::INVALID_MESSAGE, "");
          break;
        }
        if (generic_response.status() != status::OK) {
          context.set_failed(
              generic_response.application_error(),
              generic_response.error());
        } else {
          context.set_status(status::OK);
          zmq::message_t& payload = iter.next();
          if (context.response_msg) {
            if (!context.response_msg->ParseFromArray(
                    payload.data(), payload.size())) {
              context.set_failed(
                  application_error::INVALID_MESSAGE, "");
              break;
            }
          } else if (context.response_str) {
            context.response_str->assign(
                static_cast<char*>(
                    payload.data()),
                payload.size());
          }
        }
      }
      break;
    case CMSTATUS_ACTIVE:
    case CMSTATUS_INACTIVE:
    default:
      CHECK(false) << "Unexpected status: "
                   << status;
  }
  // We call signal() before we execute closure since the closure may delete
  // the rpc_controller object (which contains the sync_event).
  // XXX Check sync_event is valid. signal() before closure has no use?
  // XXX context.rpc_controller->signal();
  if (context.user_closure) {
    context.user_closure->run();
  }
}  // handle_response()

}  // namespace rpcz
