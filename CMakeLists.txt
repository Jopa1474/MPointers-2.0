cmake_minimum_required(VERSION 3.14)
project(MPointers)

set(CMAKE_CXX_STANDARD 17)

# Activa colores bonitos si es terminal
set(CMAKE_COLOR_MAKEFILE ON)

# Carpeta donde se generan los archivos protobuf
include_directories(${CMAKE_SOURCE_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR}/proto)

# Añade subdirectorios de componentes
add_subdirectory(memory_manager)
# add_subdirectory(client)
# add_subdirectory(mpointers)
