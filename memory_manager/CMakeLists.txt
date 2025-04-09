# Este archivo configura la construcción del ejecutable del servidor memory_manager.
# Incluye archivos fuente relevantes, configura las dependencias de gRPC y protobuf,
# y asegura que se incluyan los headers generados automáticamente.

cmake_minimum_required(VERSION 3.14)

# Define el ejecutable llamado memory_manager y sus archivos fuente
add_executable(memory_manager
    main.cpp                         # Entrada principal del servidor
    MemoryManagerService.cpp         # Lógica del servicio gRPC
    ../generated/memory.pb.cc        # Código generado por Protobuf (mensajes)
    ../generated/memory.grpc.pb.cc   # Código generado por Protobuf (servicios)
)

# Directorios donde buscar los headers necesarios
target_include_directories(memory_manager PRIVATE
    ../generated                     # Headers generados por protoc
    .                                # Headers locales del servidor
)

# Requiere hilos, Protobuf y gRPC
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Vinculación con bibliotecas necesarias
target_link_libraries(memory_manager
    gRPC::grpc++                     # Biblioteca principal de gRPC
    protobuf::libprotobuf            # Biblioteca principal de protobuf
    ${CMAKE_THREAD_LIBS_INIT}        # Librería de hilos del sistema
    gpr                              # gRPC runtime support library
)
