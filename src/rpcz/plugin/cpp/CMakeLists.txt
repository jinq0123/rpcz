find_package(ProtobufPlugin REQUIRED)
include_directories($PROTOBUF_INCLUDE_DIR)

set(RPCZ_SRCS rpcz_cpp_generator.cc file_generator.cc rpcz_cpp_main.cc
              rpcz_cpp_service.cc)
file(GLOB RPCZ_HEADERS *.h)

add_executable(protoc-gen-cpp_rpcz ${RPCZ_SRCS} ${RPCZ_HEADERS})
target_link_libraries(protoc-gen-cpp_rpcz optimized ${PROTOBUF_PROTOC_LIBRARY})
target_link_libraries(protoc-gen-cpp_rpcz debug ${PROTOBUF_PROTOC_LIBRARY_DEBUG})
target_link_libraries(protoc-gen-cpp_rpcz optimized ${PROTOBUF_LIBRARY})
target_link_libraries(protoc-gen-cpp_rpcz debug ${PROTOBUF_LIBRARY_DEBUG})

install(TARGETS protoc-gen-cpp_rpcz
    RUNTIME DESTINATION bin)
