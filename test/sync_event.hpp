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

#ifndef SYNC_EVENT_H
#define SYNC_EVENT_H

#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>

namespace {

// sync_event provides a mechanism for threads to wait for an event.
class sync_event {
 public:
  inline sync_event();

  // Blocks the current thread until another thread calls signal().
  inline void wait();

  // Signals that the event has occured. All threads that called wait() are
  // released.
  inline void signal();

 private:
  bool ready_;
  boost::mutex mu_;
  boost::condition_variable cond_;
};

inline sync_event::sync_event() : ready_(false) {
}

inline void sync_event::wait() {
  boost::unique_lock<boost::mutex> lock(mu_);
  while (!ready_) {
    cond_.wait(lock);
  }
}

inline void sync_event::signal() {
  boost::unique_lock<boost::mutex> lock(mu_);
  ready_ = true;
  cond_.notify_all();
}

}  // namespace
#endif
