rpcz
====

RPCZ: Protocol Buffer RPC transport

Froked from https://code.google.com/p/rpcz/

CMake
-----

If Windows have no boost dynamic libraries,
		CMake Error at D:/Program Files/CMake 2.8/share/cmake-2.8/Modules/FindBoost.cmake:1192 (message):
		  Unable to find the requested Boost libraries.
Add Entry on cmake-gui: Boost_USE_STATIC_LIBS, 
Or add line in CMakeLists.txt: SET(Boost_USE_STATIC_LIBS TRUE) 
