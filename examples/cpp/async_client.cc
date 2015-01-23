// Licensed under the Apache License, Version 2.0 (the "License");
//     http://www.apache.org/licenses/LICENSE-2.0
// Author: Jin Qing (http://blog.csdn.net/jq0123)

// Asynchronous client example.

#include <iostream>

#include "cpp/search.pb.h"
#include "cpp/search.rpcz.h"
#include "rpcz/rpc_error.hpp"
#include "rpcz/rpcz.hpp"

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
  examples::SearchResponse response;
  request.set_query("gold");

  cout << "Sending request." << endl;
  search_stub.async_Search(request, done);

  // Do other works...
  std::cin.get();
}
