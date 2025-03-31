cmake_minimum_required(VERSION 3.10)

project(MPointers-2.0)

set(CMAKE_CXX_STANDARD 17)

# Agregar directorios de encabezados
include_directories(${PROJECT_SOURCE_DIR}/include)

# Agregar el subdirectorio src
add_subdirectory(src)

