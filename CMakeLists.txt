# Archivo principal de configuración del proyecto MPointers-2.0.
# Define las opciones de compilación globales, incluye directorios compartidos
# y delega la compilación de componentes a subdirectorios.


# Requiere una versión mínima de CMake
cmake_minimum_required(VERSION 3.14)


project(MPointers)


set(CMAKE_CXX_STANDARD 17)

# Activa colores en la salida de make para una mejor visualización
set(CMAKE_COLOR_MAKEFILE ON)

# Incluye los headers generados por Protobuf (.pb.h y .grpc.pb.h)
include_directories(${CMAKE_SOURCE_DIR}/generated)

# Incluye los archivos .proto originales por si se quieren inspeccionar
include_directories(${CMAKE_SOURCE_DIR}/proto)

# Añade subdirectorios que contienen sus propios CMakeLists.txt
add_subdirectory(memory_manager)  # Servidor
add_subdirectory(client)          # Cliente