// Licensed under the Apache License, Version 2.0 (the "License");
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SYNC_CALL_HANDLER_HPP
#define RPCZ_SYNC_CALL_HANDLER_HPP

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <google/protobuf/message.h>

#include "rpcz/common.hpp"  // for scoped_ptr
#include "rpcz/rpc_error.hpp"
#include "sync_event.hpp"

namespace rpcz {

class rpc_error;

// Handler to simulate sync call.
class sync_call_handler : boost::noncopyable {
 public:
  inline explicit sync_call_handler(
      google::protobuf::Message* response)
      : response_(response) {}
  inline ~sync_call_handler(void) {}

 public:
  inline void operator()(const rpc_error* error,
      const void* data, size_t size);
  inline void wait() { sync_.wait(); }
  inline const rpc_error* get_rpc_error() const {
    return error_.get_ptr();
  }

 private:
  void handle_error(const rpc_error& err);
  void handle_invalid_message();
  inline void signal() { sync_.signal(); }

 private:
  sync_event sync_;
  google::protobuf::Message* response_;
  boost::optional<rpc_error> error_;
};

inline void sync_call_handler::operator()(
    const rpc_error* error,
    const void* data, size_t size) {
  if (error) {
    handle_error(*error);
    signal();
    return;
  }
  BOOST_ASSERT(data);
  if (NULL == response_) {
    signal();
    return;
  }
  if (response_->ParseFromArray(data, size)) {
    BOOST_ASSERT(!error_);
    signal();
    return;
  }

  // invalid message
  handle_invalid_message();
  signal();
}

}  // namespace rpcz
#endif  // RPCZ_SYNC_CALL_HANDLER_HPP
