// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_SYNC_CALL_HANDLER_HPP
#define RPCZ_SYNC_CALL_HANDLER_HPP

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include "rpcz/error_handler.hpp"
#include "rpcz/sync_event.hpp"  // XXX move to src/
#include "rpcz/common.hpp"  // for scoped_ptr

namespace rpcz {

class rpc_error;

// Handler to simulate sync call.
class sync_call_handler
{
public:
    inline explicit sync_call_handler(
        google::protobuf::Message& response);
    ~sync_call_handler(void) {}

public:
    inline void operator()(const void* data, size_t size);
    inline error_handler get_error_handler();

public:
    void wait() { state_->sync.wait(); }
    rpc_error* get_rpc_error() const { return state_->error.get(); }

private:
    void handle_error(const rpc_error& err);
    void signal() { state_->sync.signal(); }
    void handle_invalid_message();

private:
    struct state
    {
        sync_event sync;
        google::protobuf::Message* response;
        scoped_ptr<rpc_error> error;
    };
    boost::shared_ptr<state> state_;  // shared by all instance
};

inline sync_call_handler::sync_call_handler(
    google::protobuf::Message& response)
    : state_(new state)  // shared_ptr
{
    state_->response = &response;
}

inline void sync_call_handler::operator()(const void * data, size_t size)
{
    BOOST_ASSERT(data);
    if (state_->response->ParseFromArray(data, size))
    {
        BOOST_ASSERT(NULL == state_->error.get());
        signal();
        return;
    }

    // invalid message
    handle_invalid_message();
}

inline error_handler sync_call_handler::get_error_handler()
{
    return boost::bind(&sync_call_handler::handle_error, this, _1);
}

}  // namespace rpcz
#endif  // RPCZ_SYNC_CALL_HANDLER_HPP
