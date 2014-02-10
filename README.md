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
* RPCZ has been tested on Ubuntu 11.10 and Mac OS X Lion. I believe it should not be hard to get it to compile on Windows but I have not tried. 
    
CMake
-----

If Windows have no boost dynamic libraries,

    CMake Error at D:/Program Files/CMake 2.8/share/cmake-2.8/Modules/FindBoost.cmake:1192 (message):
      Unable to find the requested Boost libraries.

then add entry Boost_USE_STATIC_LIBS on cmake-gui,
or add a line in CMakeLists.txt:

    SET(Boost_USE_STATIC_LIBS TRUE) 

VC build need to check on rpcz_build_static, 
because rpcz.dll will not produce lib file,
which zsendrpc need.

