// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_INVALID_MESSAGE_ERROR_H
#define RPCZ_INVALID_MESSAGE_ERROR_H

#include <stdexcept>
#include <string>

namespace rpcz {

class invalid_message_error : public std::runtime_error {
 public:
  explicit invalid_message_error(const std::string& message)
      : std::runtime_error(message) {}
};

}  // namespace rpcz

#endif  // RPCZ_INVALID_MESSAGE_ERROR_H
