cmake_minimum_required(VERSION 3.16)
project(ClientApp)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)

add_executable(test_basic test_basic.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)
target_include_directories(test_basic PRIVATE ../generated .)
target_link_libraries(test_basic
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)


add_executable(test_gc test_gc.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)
target_include_directories(test_gc PRIVATE ../generated .)
target_link_libraries(test_gc
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)

add_executable(test_gc_coalesce test_gc_coalesce.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)
target_include_directories(test_gc_coalesce PRIVATE ../generated .)
target_link_libraries(test_gc_coalesce
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)
