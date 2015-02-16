add_executable(router router.cc)
target_link_libraries(router rpcz example_pb)
add_executable(dealer dealer.cc)
target_link_libraries(dealer rpcz example_pb)

set_property(TARGET router      PROPERTY FOLDER "Examples/full_duplex")
set_property(TARGET dealer      PROPERTY FOLDER "Examples/full_duplex")
