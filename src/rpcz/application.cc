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

#include <rpcz/application.hpp>

#include <string>
#include <zmq.hpp>

#include <rpcz/application_options.hpp>
#include <rpcz/manager.hpp>
#include <rpcz/server.hpp>

namespace rpcz {
namespace application {

void run() {
  manager::get()->run();
}

// TODO: run() in main thread, and callback in main thread.
// TODO: run_in_background() run callback in background worker thread.

void terminate() {
  manager::get()->terminate();
}

void set_worker_threads(int n) {
  application_options::set_worker_threads(n);
}

}  // namespace application
}  // namespace rpcz
