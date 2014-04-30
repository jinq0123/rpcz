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

#ifndef RPCZ_PROTO_RPC_SERVICE_HPP
#define RPCZ_PROTO_RPC_SERVICE_HPP

#include <assert.h>
#include <string>

#include <google/protobuf/descriptor.h>  // for FindMethodByName()
#include <google/protobuf/message.h>  // for Message

#include "logging.hpp"  // for INFO
#include "rpcz/common.hpp"  // for scoped_ptr
#include "rpcz/reply_context.hpp"
#include "rpcz/reply_sender.hpp"
#include "rpcz/rpc_controller.hpp"  // for application_error
#include "rpcz/rpc_service.hpp"
#include "rpcz/service.hpp"

namespace rpcz {

class proto_rpc_service : public rpc_service {
 public:
  // It will take ownership of the provided service.
  explicit proto_rpc_service(service * svc) : service_(svc) {
      assert(svc);
  }

  virtual ~proto_rpc_service() {
      delete service_;
  }

  virtual void dispatch_request(const std::string& method,
                                const void* payload, size_t payload_len,
                                const reply_context& reply_context) {
    assert(NULL != reply_context.client_connection);
    const ::google::protobuf::MethodDescriptor* descriptor =
        service_->GetDescriptor()->FindMethodByName(
            method);
    if (descriptor == NULL) {
      // Invalid method name
      DLOG(INFO) << "Invalid method name: " << method;
      reply_sender(reply_context)
          .send_error(application_error::NO_SUCH_METHOD);
      return;
    }

    scoped_ptr<google::protobuf::Message> request;
    request.reset(CHECK_NOTNULL(
        service_->GetRequestPrototype(descriptor).New()));
    if (!request->ParseFromArray(payload, payload_len)) {
      DLOG(INFO) << "Failed to parse request.";
      // Invalid proto;
      reply_sender(reply_context)
          .send_error(application_error::INVALID_MESSAGE);
      return;
    }
    service_->call_method(descriptor, *request, reply_context);
  }

 private:
  service * service_;
};

}  // namespace rpcz

#endif  // RPCZ_PROTO_RPC_SERVICE_HPP
