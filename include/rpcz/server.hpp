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

#ifndef RPCZ_SERVER_H
#define RPCZ_SERVER_H

#include <string>
#include <boost/noncopyable.hpp>
#include "rpcz/common.hpp"  // for scoped_ptr
#include "rpcz/default_service_factory.hpp"
#include "rpcz/rpcz_api.hpp"
#include "rpcz/service_factory_ptr.hpp"

namespace rpcz {
class cpp_service;
class iservice;
class server_impl;

// A server object maps incoming RPC requests to a provided service interface.
// The service interface methods are executed inside one of the worker threads.
// Non-thread-safe.
class RPCZ_API server : boost::noncopyable {
 public:
  server();
  virtual ~server();

 public:
  // Registers cpp service with this server. Only for cpp.
  // All registrations must occur before bind() is called. TODO: allow after bind()
  // The name parameter identifies the service for external clients.
  // If you use the first form, the service name from the
  //   protocol buffer definition will be used.

  // Register singleton service.
  // Singleton service means all client share the same service instance.
  // It does not take ownership of the provided service,
  //   which must be valid during server's lifetime.
  void register_singleton_service(cpp_service& svc);
  void register_singleton_service(iservice& svc, const std::string& name);

  // Register service. XXX Only for cpp service.
  // Each connection will create it's own service instance.
  template <typename Service>
  void register_service();
  template <typename Service>
  void register_service(const std::string& name);

 public:
  void bind(const std::string& endpoint);

  // TODO: unregister_service()

  // Must register service before bind.
  // Registration after bind will be ignored.

  // TODO: register after bind. Move register into broker thread.
  //       service_factory_map_ owned by broker thread.

 public:
  // service_factory creates service for each connection.
  void register_service_factory(service_factory_ptr factory, const std::string& name);

 private:
  scoped_ptr<server_impl> impl_;
};  // class server

template <typename Service>
void server::register_service() {
  register_service<Service>(Service::descriptor()->name());
}

template <typename Service>
void server::register_service(const std::string& name) {
  service_factory_ptr factory(new default_service_factory<Service>);  // shared_ptr
  register_service_factory(factory, name);
}

}  // namespace rpcz

#endif  // RPCZ_SERVER_H
