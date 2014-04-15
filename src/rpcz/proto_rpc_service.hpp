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

#include "server_impl.hpp"

#include <signal.h>
#include <string.h>
#include <functional>
#include <iostream>
#include <utility>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/stubs/common.h>
#include <zmq.hpp>

#include "client_connection.hpp"
#include "connection_manager.hpp"
#include "logging.hpp"
#include "rpcz/application.hpp"
#include "rpcz/callback.hpp"
#include "rpcz/rpc_controller.hpp"
#include "rpcz/rpc_service.hpp"
#include "rpcz/service.hpp"
#include "server_channel_impl.hpp"

namespace rpcz {

class proto_rpc_service : public rpc_service {
 public:
  // Does not take ownership of the provided service.
  explicit proto_rpc_service(service& service) : service_(service) {
  }

  virtual void dispatch_request(const std::string& method,
                               const void* payload, size_t payload_len,
                               server_channel* channel_) {
    scoped_ptr<server_channel_impl> channel(
        static_cast<server_channel_impl*>(channel_));

    const ::google::protobuf::MethodDescriptor* descriptor =
        service_.GetDescriptor()->FindMethodByName(
            method);
    if (descriptor == NULL) {
      // Invalid method name
      DLOG(INFO) << "Invalid method name: " << method,
      channel->send_error(application_error::NO_SUCH_METHOD);
      return;
    }
    channel->request_.reset(CHECK_NOTNULL(
            service_.GetRequestPrototype(descriptor).New()));
    if (!channel->request_->ParseFromArray(payload, payload_len)) {
      DLOG(INFO) << "Failed to parse request.";
      // Invalid proto;
      channel->send_error(application_error::INVALID_MESSAGE);
      return;
    }
    server_channel_impl* channel_ptr = channel.release();
    service_.call_method(descriptor,
                         *channel_ptr->request_,
                         channel_ptr);
  }

 private:
  service& service_;
};

server_impl::server_impl()
  : connection_manager_ptr_(connection_manager::get()),
	binding_(false) {
  assert(connection_manager_ptr_);
}

server_impl::~server_impl() {
  // unbind first
  BOOST_FOREACH(const bind_endpoint_set::value_type & v, endpoints_)
    connection_manager_ptr_->unbind(v);
  endpoints_.clear();
}

void server_impl::register_singleton_service(rpcz::service& service,
                                   const std::string& name) {
  if (binding_) return;
  // XXX Singleton factory...
  register_rpc_service(
      new proto_rpc_service(service), name);  // deleted in unregister_service()
}

void server_impl::register_service_factory(service_factory_ptr factory,
                                           const std::string& name) {
  assert(factory);
  if (binding_) return;  // Must register before bind().
  service_factory_map_[name] = factory;
}

void server_impl::register_rpc_service(rpcz::rpc_service* rpc_service,
                                       const std::string& name) {
  if (binding_) return;
  unregister_service(name);
  // XXX service_map_[name] = rpc_service;
}

void server_impl::unregister_service(const std::string& name) {
  // XXX
  //rpc_service_map::const_iterator it = service_map_.find(name);
  //if (it == service_map_.end()) return;
  //assert((*it).second);
  //delete (*it).second;
  //service_map_.erase(it);
}

void server_impl::bind(const std::string& endpoint) {
  binding_ = true;  // Stop registeration after bind.
  // Record endpoints for unbind later. (Server can multi bind.)
  if (!endpoints_.insert(endpoint).second)
    return;  // already bound
  connection_manager_ptr_->bind(endpoint, service_factory_map_);
}

}  // namespace rpcz
