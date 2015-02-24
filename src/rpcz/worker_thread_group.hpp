// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Worker thread group. Thread-safe.

#ifndef RPCZ_WORDER_THREAD_GROUP_HPP
#define RPCZ_WORDER_THREAD_GROUP_HPP

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread.hpp>  // for thread_group
#include <rpcz/common.hpp>  // for scoped_ptr

namespace zmq {
class context_t;
}  // namespace zmq

namespace rpcz {
class worker;

class worker_thread_group : boost::noncopyable {
 public:
  explicit worker_thread_group(int threads,
      const std::string& frontend_endpoint_,
      zmq::context_t& context_);
  // Blocks until all threads joined.
  virtual ~worker_thread_group();

 public:
  // Executes the closure on one of the worker threads.
  // XXX void add(closure* closure);

 public:
  void join_all();  // blocking

 private:
  boost::thread_group thread_group_;
  typedef scoped_ptr<worker> scoped_worker;
  boost::scoped_array<scoped_worker> workers_;
};

}  // namespace rpcz
#endif  // RPCZ_WORDER_THREAD_GROUP_HPP
