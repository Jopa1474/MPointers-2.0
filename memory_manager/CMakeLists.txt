cmake_minimum_required(VERSION 3.14)

add_executable(memory_manager
    main.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)

target_include_directories(memory_manager PRIVATE
    ../generated
)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

target_link_libraries(memory_manager
    gRPC::grpc++
    protobuf::libprotobuf
    ${CMAKE_THREAD_LIBS_INIT}
    gpr  
)
