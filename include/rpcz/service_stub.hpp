// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SERVICE_STUB_HPP
#define RPCZ_SERVICE_STUB_HPP

#include <boost/assert.hpp>
#include <google/protobuf/stubs/common.h>  // for GOOGLE_DISALLOW_EVIL_CONSTRUCTORS()
#include <rpcz/connection_ptr.hpp>
#include <rpcz/rpcz_api.hpp>

namespace rpcz {

class RPCZ_API service_stub {
 public:
  inline explicit service_stub(const connection_ptr& conn) :
      conn_(conn),  // copy
      default_timeout_ms_(-1) {
    BOOST_ASSERT(conn);
  }
  explicit service_stub(const std::string& endpoint);
  inline virtual ~service_stub() {}

 public:
  inline connection_ptr get_connection_ptr() const { return conn_; }
  inline void set_default_timeout_ms(long ms) { default_timeout_ms_ = ms; }

 protected:
  connection_ptr conn_;
  long default_timeout_ms_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(service_stub);
};

}  // namespace rpcz

#endif  // RPCZ_SERVICE_STUB_HPP
