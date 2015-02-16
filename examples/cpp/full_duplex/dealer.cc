// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)
// Example of full deplex rpc. Dealer side. Connector and responder.
// Run router and dealer to test.

#include <iostream>

#include "cpp/search.rpcz.h"
#include "rpcz/connection_ptr.hpp"

using namespace std;

namespace examples {
class SearchServiceImpl : public SearchService {
  virtual void Search(
      const SearchRequest& request,
      const rpcz::replier& rep) {
    cout << "Got request for '" << request.query() << "'" << endl;
    SearchResponse response;
    response.add_results("result1 for " + request.query());
    response.add_results("this is result2");
    rep.reply(response);
  }
};
}  // namespace examples

int main() {
  cout << "Dealer starting...";
  examples::SearchService_Stub stub("tcp://localhost:5555");

  rpcz::connection_ptr conn = stub.get_connection_ptr();
  // XXX conn.bind_service(examples::SearchServiceImpl);

  examples::SearchRequest request;  // only to trigger reversed request
  stub.async_Search(request);
  
  // Serve for router.
  std::cin.get();
}
