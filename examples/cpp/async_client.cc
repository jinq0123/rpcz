// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

// Asynchronous client example.

#include <iostream>

#include "cpp/search.rpcz.h"

using namespace std;

void done(const rpcz::rpc_error* error,
    const examples::SearchResponse& response) {
  if (error) 
    cout << error->what() << endl;
  else
    cout << response.DebugString() << endl;
}

int main() {
  examples::SearchService_Stub search_stub("tcp://localhost:5555");
  examples::SearchRequest request;
  request.set_query("gold");

  cout << "Sending request." << endl;
  search_stub.async_Search(request, done);

  // Do other works...
  std::cin.get();
}
