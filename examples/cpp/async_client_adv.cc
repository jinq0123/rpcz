// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

// Advanced asynchronous client example.
// Demonstrate all kinds of async request handlers.

#include <iostream>

#include <boost/bind.hpp>

#include "cpp/search.rpcz.h"

using namespace std;

struct context {
  int a;
};

void handler_fun(const rpcz::rpc_error* error,
    const examples::SearchResponse& response,
    context& ctx) {
  ctx.a++;
  if (error) 
    cout << error->what() << endl;
  else
    cout << response.DebugString() << endl;
  cout << ctx.a << endl;
}

struct handler {
  context ctx;

  void fun(const rpcz::rpc_error* error,
    const examples::SearchResponse& response) {
    if (error) ctx.a = 222;
    else ctx.a = response.results_size();
    cout << ctx.a << endl;
  }
};

int main() {
  examples::SearchService_Stub search_stub("tcp://localhost:5555");
  examples::SearchRequest request;
  request.set_query("gold");

  context ctx = { 123 };
  cout << "Request with context copy." << endl;
  search_stub.async_Search(request,
      boost::bind(handler_fun, _1, _2, ctx));
  cout << "Request with context reference." << endl;
  search_stub.async_Search(request,
      boost::bind(handler_fun, _1, _2, boost::ref(ctx)));

  handler hdl;
  cout << "Request with handler object." << endl;
  search_stub.async_Search(request,
      boost::bind(&handler::fun, &hdl, _1, _2));

  // Do other works...
  std::cin.get();
}
