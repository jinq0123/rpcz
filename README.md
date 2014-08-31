RPCZ: Protocol Buffer RPC transport
===================================

Forked from https://code.google.com/p/rpcz/

Introduction
------------

* RPCZ is a library for writing fast and robust RPC clients and servers that speak Protocol Buffers. 
* RPCZ currently supports writing clients and servers in C++ and Python. More languages may be added in the future. 
* The API offers both asynchronous (callbacks) and synchronous (blocking) style functionality. Both styles allow specifying deadlines in millisecond resolution. 
* RPCZ is built on top of ZeroMQ for handling the low-level I/O details in a lock-free manner. 
* The Python module is a Cython wrapper around the C++ API. 
* RPCZ has been tested on Windows, Ubuntu 11.10 and Mac OS X Lion.
    
CMake
-----

VC build need to check on rpcz_build_static, 
because rpcz.dll will not produce lib file,
which zsendrpc need.

Example
--------
### Client
See example_client.cc

		examples::SearchService_Stub search_stub("tcp://localhost:5555");
		examples::SearchRequest request;
		examples::SearchResponse response;
		request.set_query("gold");
		
		search_stub.Search(request, &response, 1000);  // timeout 1000ms
		cout << response.DebugString() << endl;

### Server
See example_server.cc

        namespace examples {
        class SearchServiceImpl : public SearchService {
          virtual void Search(const SearchRequest& request,
                              rpcz::replier replier_copy) {
            SearchResponse response;
            response.add_results("result1 for " + request.query());
            response.add_results("this is result2");
            replier_copy.send(response);
          }
        };
        }  // namespace examples
        
        int main() {
          rpcz::server server;
          server.register_service<examples::SearchServiceImpl>();
          server.bind("tcp://*:5555");
          rpcz::application::run();
        }
        