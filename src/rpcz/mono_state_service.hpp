#ifndef RPCZ_MONO_STATE_SERVICE_HPP
#define RPCZ_MONO_STATE_SERVICE_HPP

namespace rpcz {

class mono_state_service {
 public:
  mono_state_service(service & svc) : service_(svc) {};

  virtual const google::protobuf::ServiceDescriptor* GetDescriptor() {
  	return service.GetDescriptor();
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
                          server_channel* server_channel) {
    return service_.call_method(method, request, server_channel);
  }
                          
 private:
  service & service_;                          
};
}  // namespace rpcz
#endif  // RPCZ_MONO_STATE_SERVICE_HPP
