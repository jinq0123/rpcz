// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include <rpcz/application_options.hpp>

namespace rpcz {

boost::mutex application_options::mutex_;
int application_options::worker_threads_ = 1;

application_options::application_options(void) {
}

application_options::~application_options(void) {
}

void application_options::set_worker_threads(int n) {
  lock_guard lock(mutex_);
  if (n <= 0) return;
  worker_threads_ = n;
}

int application_options::get_worker_threads() {
  lock_guard lock(mutex_);
  assert(worker_threads_ > 0);
  return worker_threads_;
}

}  // namespace rpcz
