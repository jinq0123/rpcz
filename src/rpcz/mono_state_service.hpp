// Mono state service wrapping a service instance.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_MONO_STATE_SERVICE_HPP
#define RPCZ_MONO_STATE_SERVICE_HPP

#include "rpcz/service.hpp"

namespace rpcz {

// All mono_state_service instances share a same internal service.
class mono_state_service : public service {
 public:
  mono_state_service(service & svc) : service_(svc) {};

  virtual const google::protobuf::ServiceDescriptor* GetDescriptor() {
    return service_.GetDescriptor();
  }

  virtual const google::protobuf::Message& GetRequestPrototype(
      const google::protobuf::MethodDescriptor* method) const {
    return service_.GetRequestPrototype(method);
  }
  
  virtual const google::protobuf::Message& GetResponsePrototype(
      const google::protobuf::MethodDescriptor* method) const {
    return service_.GetResponsePrototype(method);
  }

  virtual void call_method(const google::protobuf::MethodDescriptor* method,
                           const google::protobuf::Message& request,
                           replier replier_copy) {
    return service_.call_method(method, request, replier_copy);
  }

 private:
  service & service_;
};
}  // namespace rpcz
#endif  // RPCZ_MONO_STATE_SERVICE_HPP
