// Punto de entrada del servidor Memory Manager.
// Este archivo se encarga de:
//  - Parsear los argumentos de línea de comandos (puerto, tamaño de memoria y carpeta de dumps)
//  - Reservar un bloque de memoria (único malloc permitido)
//  - Iniciar el servicio GRPC y quedarse escuchando peticiones

#include <iostream>
#include <string>
#include <cstdlib>
#include <grpcpp/grpcpp.h>
#include "MemoryManagerService.h"

// Imprime cómo ejecutar correctamente el programa
void printUsage() {
    std::cout << "Uso: ./memory_manager --port <PORT> --memsize <SIZE_MB> --dumpFolder <FOLDER>\n";
}

int main(int argc, char** argv) {
    // Verifica número de argumentos
    if (argc != 7) {
        printUsage();
        return 1;
    }

    int port = 0;
    int memsize_mb = 0;
    std::string dumpFolder;

    // Parseo simple de argumentos: --port, --memsize, --dumpFolder
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port") {
            port = std::stoi(argv[++i]);
        } else if (arg == "--memsize") {
            memsize_mb = std::stoi(argv[++i]);
        } else if (arg == "--dumpFolder") {
            dumpFolder = argv[++i];
        }
    }

    // Reservar bloque de memoria (único malloc permitido en el proyecto)
    size_t totalSize = memsize_mb * 1024 * 1024;
    void* memoryBlock = malloc(totalSize);
    if (!memoryBlock) {
        std::cerr << "Error reservando memoria" << std::endl;
        return 1;
    }

    std::cout << "Memoria reservada: " << totalSize << " bytes" << std::endl;

    // Configura el servidor GRPC
    std::string server_address = "0.0.0.0:" + std::to_string(port);
    MemoryManagerService service(memoryBlock, totalSize, dumpFolder);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    std::cout << "Servidor escuchando en puerto " << port << std::endl;

    // Espera indefinidamente a peticiones
    server->Wait();

    // Liberar memoria al finalizar (aunque normalmente nunca llega acá)
    free(memoryBlock);
    return 0;
}
