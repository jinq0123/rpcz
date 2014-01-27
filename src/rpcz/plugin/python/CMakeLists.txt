find_package(ProtobufPlugin REQUIRED)
include_directories($PROTOBUF_INCLUDE_DIR)

set(RPCZ_SRCS rpcz_python_generator.cc rpcz_python_main.cc)
add_executable(protoc-gen-python_rpcz ${RPCZ_SRCS})
target_link_libraries(protoc-gen-python_rpcz optimized ${PROTOBUF_LIBRARY})
target_link_libraries(protoc-gen-python_rpcz debug ${PROTOBUF_LIBRARY_DEBUG})
target_link_libraries(protoc-gen-python_rpcz optimized ${PROTOBUF_PROTOC_LIBRARY})
target_link_libraries(protoc-gen-python_rpcz debug ${PROTOBUF_PROTOC_LIBRARY_DEBUG})
    
install(TARGETS protoc-gen-python_rpcz
    RUNTIME DESTINATION bin)
