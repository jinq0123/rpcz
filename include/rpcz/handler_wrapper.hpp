// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_HANDLER_WRAPPER_HPP
#define RPCZ_HANDLER_WRAPPER_HPP

namespace rpcz {

// Wrap specific response handler type to response_message_handler.
// Only for C++?
// Response should be subtype of protocol::Message.
// The input handler will be copied.
template <typename Response>
struct handler_wrapper
{
public:
  typedef boost::function<void (const Response&)> handler;

public:
  explicit handler_wrapper(const handler& hdl) : handler_(hdl) {
  }

public:
  // Returns false if message is illegal.
  inline bool operator()(const void * data, size_t size);

private:
  handler handler_;
};

// Returns false if message is illegal.
template <typename Response>
inline bool handler_wrapper<Response>::operator()(
    const void * data, size_t size) {
  BOOST_ASSERT(data);
  if (!handler_)
    return true;  // ignore message

  Response resp;
  if (!resp.ParseFromArray(data, size))
    return false;  // illegal message

  handler_(resp);
  return true;
}  // operator()()

}  // namespace rpcz

#endif  // RPCZ_HANDLER_WRAPPER_HPP
