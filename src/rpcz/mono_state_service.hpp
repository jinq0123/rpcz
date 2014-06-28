// Mono state service wrapping a service instance.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_MONO_STATE_SERVICE_HPP
#define RPCZ_MONO_STATE_SERVICE_HPP

#include "rpcz/iservice.hpp"

namespace rpcz {

// All mono_state_service instances share a same internal service.
class mono_state_service : public iservice {
 public:
  mono_state_service(iservice & svc) : service_(svc) {}
  virtual ~mono_state_service() {}

 public:
  virtual void dispatch_request(const std::string& method,
                                const void* payload, size_t payload_len,
                                replier replier_copy)
  {
    service_.dispatch_request(method, payload, payload_len, replier_copy);
  }

 private:
  iservice & service_;
};  // class mono_state_service
}  // namespace rpcz
#endif  // RPCZ_MONO_STATE_SERVICE_HPP
