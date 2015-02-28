// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Dynamic singleton helper for manager class.
// See struct manager::dyn_singleton_helper in 
//  "(A dynamic) Singleton using weak_ptr and shared_ptr"
//  http://boost.2283326.n4.nabble.com/A-dynamic-Singleton-using-weak-ptr-and-shared-ptr-td2581447.html

#include <rpcz/manager.hpp>

namespace rpcz {

manager::dyn_singleton_helper::manager_weak_ptr
    manager::dyn_singleton_helper::mgr_wptr;
boost::recursive_mutex
    manager::dyn_singleton_helper::mgr_wptr_mtx;

bool manager::dyn_singleton_helper::mgr_exists = false;
boost::recursive_mutex manager::dyn_singleton_helper::mgr_exists_mtx;
boost::condition_variable_any manager::dyn_singleton_helper::cond;

struct manager::dyn_singleton_deleter {
  void operator()(manager* p) {
    BOOST_ASSERT(p);
    delete p;
    manager::dyn_singleton_helper::finish_destruction();
  }
};

void manager::dyn_singleton_helper::start_construction()
{
  boost::recursive_mutex::scoped_lock lock(mgr_exists_mtx);
  while (mgr_exists) {
    cond.wait(lock);
  }
  mgr_exists = true;
}

void manager::dyn_singleton_helper::finish_destruction()
{
  boost::recursive_mutex::scoped_lock lock(mgr_exists_mtx);
  mgr_exists = false;
  cond.notify_one();
}

manager_ptr manager::dyn_singleton_helper::make_manager_ptr() {
  start_construction();
  manager_ptr p(new manager(), manager::dyn_singleton_deleter());  // shared_ptr
  mgr_wptr = p;
  return p;
}

}  // namespace rpcz
