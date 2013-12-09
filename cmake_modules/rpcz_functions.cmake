set(RPCZ_PLUGIN_ROOT ${CMAKE_BINARY_DIR}/src/rpcz/plugin)

function(PROTOBUF_GENERATE_RPCZ SRCS HDRS)
  if(MSVC)
    # Only support msvc Release version.
    set(PLUGIN_BIN ${RPCZ_PLUGIN_ROOT}/cpp/Release/protoc-gen-cpp_rpcz.exe)
  else(MSVC)
    set(PLUGIN_BIN ${RPCZ_PLUGIN_ROOT}/cpp/protoc-gen-cpp_rpcz)
  endif(MSVC)
  
  PROTOBUF_GENERATE_MULTI(PLUGIN "cpp_rpcz" PROTOS ${ARGN}
                          OUTPUT_STRUCT "_SRCS:.rpcz.cc;_HDRS:.rpcz.h"
                          FLAGS "--plugin=protoc-gen-cpp_rpcz=${PLUGIN_BIN}"
                          DEPENDS ${PLUGIN_BIN})
  set(${SRCS} ${_SRCS} PARENT_SCOPE)
  set(${HDRS} ${_HDRS} PARENT_SCOPE)
endfunction()

function(PROTOBUF_GENERATE_PYTHON_RPCZ SRCS)
  if(MSVC)
    # Only support msvc Release version.
    set(PLUGIN_BIN ${RPCZ_PLUGIN_ROOT}/python/Release/protoc-gen-python_rpcz.exe)
  else(MSVC)
    set(PLUGIN_BIN ${RPCZ_PLUGIN_ROOT}/python/protoc-gen-python_rpcz)
  endif(MSVC)
  
  PROTOBUF_GENERATE_MULTI(PLUGIN "python_rpcz" PROTOS ${ARGN}
                          OUTPUT_STRUCT "_SRCS:_rpcz.py"
                          FLAGS "--plugin=protoc-gen-python_rpcz=${PLUGIN_BIN}"
                          DEPENDS ${PLUGIN_BIN})
  set(${SRCS} ${_SRCS} PARENT_SCOPE)
endfunction()
