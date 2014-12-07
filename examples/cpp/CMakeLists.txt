include(rpcz_functions)
find_package(ProtobufPlugin REQUIRED)
PROTOBUF_GENERATE_CPP(SEARCH_PB_SRCS SEARCH_PB_HDRS ../common/search.proto)
PROTOBUF_GENERATE_RPCZ(SEARCH_RPCZ_SRCS SEARCH_RPCZ_HDRS ../common/search.proto)

add_library(example_pb ${SEARCH_PB_SRCS} ${SEARCH_PB_HDRS} ${SEARCH_RPCZ_SRCS}
                      ${SEARCH_RPCZ_HDRS})
target_link_libraries(example_pb optimized ${PROTOBUF_LIBRARY})
target_link_libraries(example_pb debug ${PROTOBUF_LIBRARY_DEBUG})
include_directories(${Rpcz_BINARY_DIR}/examples)

add_executable(example_client example_client.cc)
target_link_libraries(example_client rpcz example_pb)

add_executable(example_server example_server.cc)
target_link_libraries(example_server rpcz example_pb)

set_property(TARGET example_pb     PROPERTY FOLDER "Examples")
set_property(TARGET example_client PROPERTY FOLDER "Examples")
set_property(TARGET example_server PROPERTY FOLDER "Examples")
