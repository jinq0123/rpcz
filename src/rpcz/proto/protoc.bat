REM protoc rpcz.proto and move to include and src dir.
REM Run after rpcz.proto changed.
REM Need protoc.exe in system path.
protoc.exe --cpp_out . rpcz.proto
move rpcz.pb.h ../../../include/rpcz/
move rpcz.pb.cc ../
