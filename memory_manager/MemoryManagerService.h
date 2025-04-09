// Declaración de la clase MemoryManagerService, que implementa el servicio gRPC
// definido en memory.proto. Esta clase expone los métodos remotos que permiten
// a un cliente crear bloques de memoria, leer/escribir valores, manejar referencias
// y dejar que el Garbage Collector libere bloques automáticamente.


#ifndef MEMORY_MANAGER_SERVICE_H
#define MEMORY_MANAGER_SERVICE_H

#include <grpcpp/grpcpp.h>
#include "../generated/memory.grpc.pb.h"

#include "MemoryBlock.h"
#include "MemoryTable.h"

#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <string>

// Servicio que implementa las operaciones remotas sobre la memoria
class MemoryManagerService final : public MemoryManager::Service {
public:
    // Constructor: recibe la memoria pre-reservada y la carpeta de dumps
    MemoryManagerService(void* memoryBase, size_t totalSize, const std::string& dumpFolder);

    // Destructor: finaliza el hilo del Garbage Collector
    ~MemoryManagerService();

    // gRPC: Crear un nuevo bloque en memoria
    grpc::Status Create(grpc::ServerContext* context,
                        const CreateRequest* request,
                        CreateResponse* response) override;

    // gRPC: Escribir un valor en un bloque de memoria remoto
    grpc::Status Set(grpc::ServerContext* context,
                     const SetRequest* request,
                     Empty* response) override;

    // gRPC: Leer el valor de un bloque remoto
    grpc::Status Get(grpc::ServerContext* context,
                     const GetRequest* request,
                     GetResponse* response) override;

    // gRPC: Incrementar el contador de referencias (refCount++)
    grpc::Status IncreaseRefCount(grpc::ServerContext* context,
                                  const IdRequest* request,
                                  Empty* response) override;

    // gRPC: Decrementar el contador de referencias (refCount--)
    grpc::Status DecreaseRefCount(grpc::ServerContext* context,
                                  const IdRequest* request,
                                  Empty* response) override;

private:
    // Puntero a la memoria reservada al inicio
    void* memoryStart;

    // Tamaño total en bytes del bloque de memoria reservado
    size_t memorySize;

    // Carpeta donde se guardan los archivos dump
    std::string dumpDirectory;

    // Protege acceso concurrente a memoria
    std::mutex memMutex;

    // Tabla principal de bloques de memoria en uso
    MemoryTable table;

    // Hilo de recolección de basura
    std::thread gcThread;

    // Bandera de ejecución del GC
    std::atomic<bool> runningGC;

    // Método privado que corre el garbage collector en background
    void runGarbageCollector();
};

#endif // MEMORY_MANAGER_SERVICE_H
