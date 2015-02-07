// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Shared pointer of ichannel.

#ifndef RPCZ_CHANNEL_PTR_HPP
#define RPCZ_CHANNEL_PTR_HPP

#include <boost/shared_ptr.hpp>

namespace rpcz {

class ichannel;
typedef boost::shared_ptr<ichannel> channel_ptr;

}  // namespace rpcz

#endif  // RPCZ_CHANNEL_PTR_HPP
