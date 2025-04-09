# Este archivo configura la construcción de todos los programas cliente que prueban
# la funcionalidad del sistema MPointers.
#
# Cada ejecutable representa una prueba distinta del sistema, desde lo básico
# hasta pruebas de Garbage Collector, coalescing y lista enlazada.
#
# Todos los ejecutables se compilan incluyendo los archivos generados de gRPC
# (memory.pb.cc, memory.grpc.pb.cc) y enlazan contra las bibliotecas necesarias
# para ejecutar gRPC y protobuf correctamente.

cmake_minimum_required(VERSION 3.16)
project(ClientApp)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)

# ==================== TEST: Básico ====================
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

# ==================== TEST: Garbage Collector ====================
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

# ==================== TEST: Coalescing ====================
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

# ==================== TEST: MPointer básico ====================
add_executable(test_mpointer test_mpointer.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)
target_include_directories(test_mpointer PRIVATE ../generated ../mpointers .)
target_link_libraries(test_mpointer
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)

# ==================== TEST: Asignación entre MPointers ====================
add_executable(test_mpointer_assign test_mpointer_assign.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)
target_include_directories(test_mpointer_assign PRIVATE ../generated ../mpointers .)
target_link_libraries(test_mpointer_assign
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)

# ==================== TEST: Lista Enlazada con MPointer ====================
add_executable(test_linked_list test_linked_list.cpp
    ../generated/memory.pb.cc
    ../generated/memory.grpc.pb.cc
)
target_include_directories(test_linked_list PRIVATE ../generated ../mpointers .)
target_link_libraries(test_linked_list
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    gpr
)
