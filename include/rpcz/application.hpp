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

#ifndef RPCZ_APPLICATION_HPP
#define RPCZ_APPLICATION_HPP

#include <rpcz/rpcz_api.hpp>

namespace rpcz {

// rpcz::application is a simple interface that helps setting up a common
// RPCZ client or server application.
// Thread-safe.
namespace application {

  // Blocks the current thread until another thread calls terminate.
  RPCZ_API void run();

  // Terminate the rpcz application internal threads.
  // Releases all the threads that are blocked inside run().
  RPCZ_API void terminate();

  // Set number of worker threads. Those threads are used for running user code:
  //   handling server requests or running callbacks.
  // Default 1.
  // Should set BEFORE any client or server.
  RPCZ_API void set_worker_threads(int n);
}  // namespace application

}  // namespace rpcz
#endif  // RPCZ_APPLICATION_HPP
