// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Dynamic singleton helper for manager class.
// See struct dynamic_singleton::impl in 
//  "(A dynamic) Singleton using weak_ptr and shared_ptr"
//  http://boost.2283326.n4.nabble.com/A-dynamic-Singleton-using-weak-ptr-and-shared-ptr-td2581447.html

#ifndef RPCZ_MANAGER_DYN_SINGLETON_HELPER_HPP
#define RPCZ_MANAGER_DYN_SINGLETON_HELPER_HPP

// included in manager.hpp

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>

#include <rpcz/manager_ptr.hpp>

namespace rpcz {

struct manager::dyn_singleton_helper : private boost::noncopyable {
  static void start_construction();
  static void finish_destruction();

  typedef boost::weak_ptr<manager> manager_weak_ptr;
  static manager_weak_ptr mgr_wptr;
  // mutex protecting mgr_wptr
  static boost::recursive_mutex mgr_wptr_mtx;

  static bool mgr_exists;
  // mutex protecting mgr_exists
  static boost::recursive_mutex mgr_exists_mtx;

  static boost::condition_variable_any cond;

  static inline manager_ptr get_manager_ptr();

private:
  static manager_ptr make_manager_ptr();

private:
  // no constructing
  dyn_singleton_helper() {}
  ~dyn_singleton_helper() {}
};

inline manager_ptr manager::dyn_singleton_helper::get_manager_ptr() {
  // Syncronise Initialization:
  boost::recursive_mutex::scoped_lock lock(
      dyn_singleton_helper::mgr_wptr_mtx);

  // Acquire singleton pointer:
  manager_ptr p = dyn_singleton_helper::mgr_wptr.lock();
  if (p) return p;

  return make_manager_ptr();
}

}  // namespace rpcz
#endif  // RPCZ_MANAGER_DYN_SINGLETON_HELPER_HPP
