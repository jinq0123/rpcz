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

