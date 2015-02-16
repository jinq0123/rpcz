// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Example of full duplex rpc. Router side. Binder and requester.
// Run router and dealer to test.

#include <iostream>

#include "cpp/search.rpcz.h"

using namespace std;
using namespace examples;

void done(const rpcz::rpc_error* error,
    const examples::SearchResponse& response) {
  if (error) 
    cout << error->what() << endl;
  else
    cout << response.DebugString() << endl;
}

// Test reversed request.
class ServiceImpl : public SearchService {
  virtual void Search(
      const SearchRequest& /*request*/,
      const rpcz::replier& rep) {
    cout << "Got request from dealer." << endl;

    examples::SearchService_Stub search_stub(rep.get_connection_ptr());
    examples::SearchRequest request;
    request.set_query("gold");
    cout << "Sending request." << endl;
    search_stub.async_Search(request, done);
  }
};

int main() {
  rpcz::server server;
  server.register_service<ServiceImpl>();
  cout << "Router on port 5555." << endl;
  server.bind("tcp://*:5555");
  rpcz::application::run();
}
