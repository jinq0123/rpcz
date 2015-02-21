#ifndef RPCZ_APPLICATION_OPTIONS_H
#define RPCZ_APPLICATION_OPTIONS_H

#include <boost/thread.hpp>

namespace zmq {
class context_t;
}  // namespace zmq

namespace rpcz {

// Thread-safe.
class application_options
{
public:
  application_options(void);
  ~application_options(void);

public:
  static void set_worker_threads(int n);
  static int get_worker_threads();

 private:
  static boost::mutex mutex_;
  typedef boost::lock_guard<boost::mutex> lock_guard;

 private:
  // Number of worker threads. Those threads are used for
  // running user code: handling server requests or running callbacks.
  static int worker_threads_;  // default 1
};  // class application_options

}  // namespace rpcz

#endif  // RPCZ_APPLICATION_OPTIONS_H
