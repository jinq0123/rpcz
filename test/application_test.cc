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

#include <zmq.hpp>
#include "rpcz/application.hpp"
#include "rpcz/manager.hpp"
#include "gtest/gtest.h"

namespace rpcz {

class application_test : public ::testing::Test {
};

TEST_F(application_test, InitializesWithProvidedZeroMQContext) {
  scoped_ptr<zmq::context_t> context(new zmq::context_t(1));
  {
    ASSERT_TRUE(manager::is_destroyed());
    application::set_zmq_context(context.get());
    connection_manager_ptr cm = manager::get();
  }
  context.reset();
}

}  // namespace rpcz


