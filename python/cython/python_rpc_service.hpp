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

#ifndef ZRPC_RPC_SERVICE_H
#define ZRPC_RPC_SERVICE_H

#include <string>
#include "Python.h"

#include "rpcz/iservice.hpp"
#include "rpcz/replier.hpp"

namespace rpcz {

// Service that helps forwarding the requests to Python-land.
// Adaptor from Python user_service to Cpp iservice.
// TODO: Rename to python_service like cpp_service
class PythonRpcService : public iservice {
 public:
  typedef void(*Handler)(PyObject* user_service, std::string& method,
                         void* payload, size_t payload_len,
                         replier replier_copy);  // rpc_handler_bridge()

  PythonRpcService(Handler handler, PyObject *user_service)
      : user_service_(user_service), handler_(handler) {
        Py_INCREF(user_service_);
  }

  ~PythonRpcService() {
    Py_DECREF(user_service_);
  }

  virtual void dispatch_request(const std::string& method,
                                const void* payload, size_t payload_len,
                                replier replier_copy) {
    // Call rpc_handler_bridge() -> user_service._call_method(...)
    handler_(user_service_,
             *const_cast<std::string*>(&method),  // TODO: Need const_cast?
             const_cast<void*>(payload), payload_len, replier_copy);
  }

 private:
  PyObject *user_service_;  // User defined service.
  Handler handler_;  // = rpc_handler_bridge()
};
}  // namespace rpcz
#endif
