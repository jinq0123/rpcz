# Unit tests use rpcz_static lib.
add_definitions(-DRPCZ_STATIC)

add_subdirectory(proto)
set(CTEST_OUTPUT_ON_FAILURE 1)

include_directories(${Rpcz_BINARY_DIR}/test)
include_directories(${Rpcz_SOURCE_DIR}/src)
add_library(rpcz_gtest_main rpcz_gtest_main.cc)

include(build_deps)

if(MSVC)

# Set gtest glog include dir and library in CMake UI.
set(GLOG_INCLUDE_DIR "" CACHE PATH "GLog include dir")
set(GLOG_LIBRARY_DEBUG "" CACHE FILEPATH "GLog debug library")
set(GLOG_LIBRARY_RELEASE "" CACHE FILEPATH "GLog release library")
set(GTEST_INCLUDE_DIR "" CACHE PATH "GTest include dir")
set(GTEST_LIBRARY_DEBUG "" CACHE FILEPATH "GTest debug library")
set(GTEST_LIBRARY_RELEASE "" CACHE FILEPATH "GTest release library")
include_directories(${GLOG_INCLUDE_DIR})
include_directories(${GTEST_INCLUDE_DIR})    

else(MSVC)

BuildDependency(
    GLog
    http://google-glog.googlecode.com/files/glog-0.3.1-1.tar.gz
    GLOG
    libglog)

ExternalProject_Add(
    GTest
    URL http://googletest.googlecode.com/files/gtest-1.6.0.zip
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(GTest binary_dir)
ExternalProject_Get_Property(GTest source_dir)
include_directories(${source_dir}/include)    
link_directories(${binary_dir})

endif(MSVC)

function(rpcz_test TESTNAME)
  CMAKE_PARSE_ARGUMENTS(OPTIONS "" "" "SRCS;LIBS" ${ARGN})
  add_executable(${TESTNAME} ${OPTIONS_SRCS})
  set_property(TARGET ${TESTNAME} PROPERTY FOLDER "Tests")
  set(LIBS rpcz_static rpcz_gtest_main
  )
if(NOT MSVC)
  list(APPEND LIBS pthread
      gtest ${GLOG_LIBRARIES}
  )
endif(NOT MSVC)

  list(APPEND LIBS ${OPTIONS_LIBS})
  target_link_libraries(${TESTNAME} ${LIBS})
if(MSVC)
  target_link_libraries(${TESTNAME} optimized ${GLOG_LIBRARY_RELEASE})
  target_link_libraries(${TESTNAME} optimized ${GTEST_LIBRARY_RELEASE})
  target_link_libraries(${TESTNAME} debug ${GLOG_LIBRARY_DEBUG})
  target_link_libraries(${TESTNAME} debug ${GTEST_LIBRARY_DEBUG})
endif(MSVC)  
  add_test(${TESTNAME} ${TESTNAME})
endfunction()

rpcz_test(callback_test SRCS callback_test.cc)
rpcz_test(manager_test SRCS manager_test.cc)
rpcz_test(client_server_test SRCS client_server_test.cc LIBS search_pb)
rpcz_test(request_interface_test SRCS request_interface_test.cc LIBS search_pb)
