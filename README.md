RPCZ: Protocol Buffer RPC transport
===================================

Forked from https://code.google.com/p/rpcz/

Introduction
------------

* RPCZ is a library for writing fast and robust RPC clients and servers that speak Protocol Buffers. 
* RPCZ currently supports writing clients and servers in C++ and Python. More languages may be added in the future. 
* The API offers both asynchronous (callbacks) and synchronous (blocking) style functionality. Both styles allow specifying deadlines in millisecond resolution. 
* RPCZ supports full duplex RPC over the same connection.
* Each connection has its service instance,
  or all connections share one service instance. (TODO: example)
* RPCZ is built on top of ZeroMQ for handling the low-level I/O details in a lock-free manner. 
* The Python module is a Cython wrapper around the C++ API. 
* RPCZ has been tested on Windows, Ubuntu 11.10 and Mac OS X Lion.

Dependings
-----------
RPCZ depends on:
* boost
* protobuf
* zeromq
* cppzmq
* glog
* gtest
    
Example
--------
### Sync Client
See sync_client.cc

    SearchService_Stub search_stub("tcp://localhost:5555");
    SearchRequest request;
    request.set_query("gold");
    SearchResponse response = search_stub.Search(request);

### Server
See example_server.cc

    class SearchServiceImpl : public SearchService {
      virtual void Search(const SearchRequest& request,
                          rpcz::replier replier_copy) {
        SearchResponse response;
        response.add_results("result1 for " + request.query());
        response.add_results("this is result2");
        replier_copy.send(response);
      }
    };
    
    int main() {
      rpcz::server svr;
      svr.register_service<SearchServiceImpl>();
      svr.bind("tcp://*:5555");
      rpcz::application::run();
    }

### Async Client
See async_client.cc

    void done(const rpcz::rpc_error* error,
        const SearchResponse& response) {
      if (error) 
        cout << error->what() << endl;
      else
        cout << response.DebugString() << endl;
    }
    
    int main() {
      SearchService_Stub search_stub("tcp://localhost:5555");
      SearchRequest request;
      request.set_query("gold");
    
      search_stub.async_Search(request, done);
    
      // Do other works...
      cin.get();
    }
    
### Async Handlers Using boost::bind
See async_client_adv.cc

    context ctx = { 123 };
    search_stub.async_Search(request,
        boost::bind(handler_fun, _1, _2, ctx));
    search_stub.async_Search(request,
        boost::bind(handler_fun, _1, _2, boost::ref(ctx)));
  
    handler hdl;
    search_stub.async_Search(request,
        boost::bind(&handler::fun, &hdl, _1, _2));

### Full Duplex RPC
See full_duplex/dealer.cc, router.cc

Register service on stub's connection:

    SearchService_Stub stub("tcp://localhost:5555");
    rpcz::connection_ptr conn = stub.get_connection_ptr();
    conn->register_service(SearchServiceImpl::descriptor()->name(),
        new SearchServiceImpl);

Get connection from the replier and create stub:

    virtual void Search(
        const SearchRequest& /*request*/,
        const rpcz::replier& rep) {
      SearchService_Stub search_stub(rep.get_connection_ptr());
      ...
    }

Thread-Safety
-------------
Service stub and connection are thread-safe
and can be shared and used in any threads.
server class is not thread-safe.
The callbacks will be run in multiple internal worker threads.
Worker threads number can be set by application::set_worker_threads(), default 1.
One connection's callbacks will always run in the same worker thread.
