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

#include "rpcz/common.hpp"  // for scoped_ptr
#include "rpcz/rpc_service.hpp"
#include "rpcz/service.hpp"
#include "server_channel_impl.hpp"

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
                               server_channel* channel_) {
    scoped_ptr<server_channel_impl> channel(
        static_cast<server_channel_impl*>(channel_));

    const ::google::protobuf::MethodDescriptor* descriptor =
        service_->GetDescriptor()->FindMethodByName(
            method);
    if (descriptor == NULL) {
      // Invalid method name
      DLOG(INFO) << "Invalid method name: " << method,
      channel->send_error(application_error::NO_SUCH_METHOD);
      return;
    }
    channel->request_.reset(CHECK_NOTNULL(
            service_->GetRequestPrototype(descriptor).New()));
    if (!channel->request_->ParseFromArray(payload, payload_len)) {
      DLOG(INFO) << "Failed to parse request.";
      // Invalid proto;
      channel->send_error(application_error::INVALID_MESSAGE);
      return;
    }
    server_channel_impl* channel_ptr = channel.release();
    service_->call_method(descriptor,
                         *channel_ptr->request_,
                         channel_ptr);
  }

 private:
  service * service_;
};

}  // namespace rpcz

#endif  // RPCZ_PROTO_RPC_SERVICE_HPP
