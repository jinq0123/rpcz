
# The version number.
set (RPCZ_VERSION_MAJOR 0)
set (RPCZ_VERSION_MINOR 9)

add_subdirectory(plugin)

protobuf_generate_cpp(RPCZ_PB_SRCS RPCZ_PB_HDRS proto/rpcz.proto)
set(PROTO_SOURCES ${RPCZ_PB_SRCS} ${RPCZ_PB_HDRS})

include_directories(${Rpcz_BINARY_DIR}/src)
include_directories(${Rpcz_SOURCE_DIR}/src)

set(RPCZ_SOURCES
    application.cc
    application_options.cc
    assert_application_error_code.cc
    broker_thread.cc
    clock.cc
    connection.cc
    cpp_service.cc
    event_id_generator.cc
    manager.cc
    reactor.cc
    request_handler.cc
    request_handler_manager.cc
    rpc_controller.cc
    rpc_error.cc
    server.cc
    server_impl.cc
    service_stub.cc
    singleton_service_factory.cc
    sync_call_handler.cc
    sync_event.cc
    sync_requester.cc
    worker_thread_fun.cc
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

add_library(rpcz_static STATIC ${RPCZ_SOURCES})
target_link_libraries(rpcz_static ${RPCZ_LIB_DEPS})
target_link_libraries(rpcz_static optimized ${PROTOBUF_LIBRARY})
target_link_libraries(rpcz_static debug ${PROTOBUF_LIBRARY_DEBUG})
set_target_properties(rpcz_static PROPERTIES COMPILE_DEFINITIONS "RPCZ_STATIC")
list(APPEND RPCZ_TARGET_LIBS rpcz_static)

add_executable(zsendrpc zsendrpc.cc)
target_link_libraries(zsendrpc rpcz ${Boost_PROGRAM_OPTIONS_LIBRARIES})

install(TARGETS ${RPCZ_TARGET_LIBS} zsendrpc
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
