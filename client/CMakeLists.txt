add_executable(client_app main.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)

target_include_directories(client_app PRIVATE
    ../generated
    .
)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(client_app
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)

add_executable(test_gc test_gc.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)

target_include_directories(test_gc PRIVATE
    ../generated
    .
)

target_link_libraries(test_gc
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)
