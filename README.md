rpcz
====

RPCZ: Protocol Buffer RPC transport

Forked from https://code.google.com/p/rpcz/

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

