// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Assert applicatioin error code values.

#include <boost/static_assert.hpp>
#include <rpcz/application_error_code.hpp>
#include <rpcz/rpcz.pb.h>

namespace rpcz {
BOOST_STATIC_ASSERT(application_error::INVALID_HEADER == rpc_response_header::INVALID_HEADER);
BOOST_STATIC_ASSERT(application_error::NO_SUCH_SERVICE == rpc_response_header::NO_SUCH_SERVICE);
BOOST_STATIC_ASSERT(application_error::NO_SUCH_METHOD == rpc_response_header::NO_SUCH_METHOD);
BOOST_STATIC_ASSERT(application_error::INVALID_MESSAGE == rpc_response_header::INVALID_MESSAGE);
BOOST_STATIC_ASSERT(application_error::METHOD_NOT_IMPLEMENTED == rpc_response_header::METHOD_NOT_IMPLEMENTED);
BOOST_STATIC_ASSERT(application_error::TIMEOUT_EXPIRED == rpc_response_header::TIMEOUT_EXPIRED);

// warning LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
char _ASSERT_APPLICATION_ERROR_CODE_CC_[] = "";
}  // namespace rpcz
