// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Assert applicatioin error code values.

#include <boost/static_assert.hpp>
#include <rpcz/application_error_code.hpp>
#include <rpcz/rpcz.pb.h>

namespace rpcz {
BOOST_STATIC_ASSERT(application_error::APPLICATION_NO_ERROR == rpc_response_header::APPLICATION_NO_ERROR);
BOOST_STATIC_ASSERT(application_error::INVALID_HEADER == rpc_response_header::INVALID_HEADER);
BOOST_STATIC_ASSERT(application_error::NO_SUCH_SERVICE == rpc_response_header::NO_SUCH_SERVICE);
BOOST_STATIC_ASSERT(application_error::NO_SUCH_METHOD == rpc_response_header::NO_SUCH_METHOD);
BOOST_STATIC_ASSERT(application_error::INVALID_MESSAGE == rpc_response_header::INVALID_MESSAGE);
BOOST_STATIC_ASSERT(application_error::METHOD_NOT_IMPLEMENTED == rpc_response_header::METHOD_NOT_IMPLEMENTED);
}  // namespace rpcz
