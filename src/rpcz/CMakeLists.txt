project (RPCZ)

# The version number.
set (RPCZ_VERSION_MAJOR 0)
set (RPCZ_VERSION_MINOR 9)

add_subdirectory(plugin)

protobuf_generate_cpp(RPCZ_PB_SRCS RPCZ_PB_HDRS proto/rpcz.proto)
set(PROTO_SOURCES ${RPCZ_PB_SRCS} ${RPCZ_PB_HDRS})

include_directories(${PROJECT_BINARY_DIR})
set(RPCZ_SOURCES
    application.cc
    application_options.cc
    broker_thread.cc
    client_connection.cc
    clock.cc
    connection.cc
    connection_manager.cc 
    event_id_generator.cc
    reactor.cc
    request_handler.cc
    request_handler_manager.cc
    rpc_controller.cc
    rpc_channel.cc
    rpc_channel_impl.cc
    server.cc
    server_impl.cc
    sync_event.cc
    zmq_utils.cc
    ${PROTO_SOURCES})
file(GLOB RPCZ_HEADERS *.hpp)
set(RPCZ_LIB_DEPS ${ZeroMQ_LIBRARIES}
                  ${Boost_CHRONO_LIBRARIES}
                  ${Boost_DATE_TIME_LIBRARIES}
                  ${Boost_SYSTEM_LIBRARIES}
                  ${Boost_THREAD_LIBRARIES})

add_library(rpcz SHARED ${RPCZ_SOURCES} ${RPCZ_HEADERS} ${RPCZ_PUBLIC_HEADERS})
target_link_libraries(rpcz ${RPCZ_LIB_DEPS})
target_link_libraries(rpcz optimized ${PROTOBUF_LIBRARY})
target_link_libraries(rpcz debug ${PROTOBUF_LIBRARY_DEBUG})

set(RPCZ_TARGET_LIBS rpcz)

if(rpcz_build_static)
    add_library(rpcz_static STATIC ${RPCZ_SOURCES})
    target_link_libraries(rpcz_static ${RPCZ_LIB_DEPS})
    set_target_properties(rpcz_static PROPERTIES OUTPUT_NAME rpcz)
    list(APPEND RPCZ_TARGET_LIBS rpcz_static)
endif(rpcz_build_static)

add_executable(zsendrpc zsendrpc.cc)
target_link_libraries(zsendrpc rpcz ${Boost_PROGRAM_OPTIONS_LIBRARIES})

if(rpcz_build_static)
    add_dependencies(zsendrpc rpcz_static)  # zsendrpc need rpcz.lib
endif(rpcz_build_static)

install(TARGETS ${RPCZ_TARGET_LIBS} zsendrpc
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
