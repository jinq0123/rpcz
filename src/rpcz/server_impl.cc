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

#include <boost/foreach.hpp>

#include "connection_manager.hpp"
#include "proto_rpc_service.hpp"

namespace rpcz {

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

// DEL
//void server_impl::register_singleton_service(rpcz::service& service,
//                                             const std::string& name) {
//  if (binding_) return;
//  // XXX Singleton factory...
//  register_rpc_service(
//      new proto_rpc_service(service), name);  // deleted in unregister_service()
//}

void server_impl::register_service_factory(service_factory_ptr factory,
                                           const std::string& name) {
  assert(factory);
  if (binding_) return;  // Must register before bind().
  service_factory_map_[name] = factory;
}

void server_impl::register_rpc_service(rpcz::rpc_service* rpc_service,
                                       const std::string& name) {
  if (binding_) return;
  // unregister_service(name);
  // XXX service_map_[name] = rpc_service;
}

void server_impl::bind(const std::string& endpoint) {
  binding_ = true;  // Stop registeration after bind.
  // Record endpoints for unbind later. (Server can multi bind.)
  if (!endpoints_.insert(endpoint).second)
    return;  // already bound
  connection_manager_ptr_->bind(endpoint, service_factory_map_);
}

}  // namespace rpcz
