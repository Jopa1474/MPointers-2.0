// Esta prueba evalúa si el Garbage Collector libera correctamente bloques
// con refcount 0 y si el sistema de memoria realiza la fusión (coalescing)
// de bloques libres contiguos.
// El test crea tres bloques consecutivos, los libera en un orden que activa
// la lógica de coalescing y finalmente genera un dump que se puede revisar
// para verificar que se fusionaron en un único hueco libre.

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include "../generated/memory.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace std::chrono_literals;

// Cliente gRPC que interactúa con el servidor de MemoryManager
class MemoryClient {
public:
    // Constructor con canal gRPC
    MemoryClient(std::shared_ptr<Channel> channel)
        : stub(MemoryManager::NewStub(channel)) {}

    // Solicita la creación de un nuevo bloque
    uint32_t Create(uint32_t size, const std::string& type) {
        CreateRequest req;
        req.set_size(size);
        req.set_type(type);
        CreateResponse resp;
        ClientContext ctx;
        Status status = stub->Create(&ctx, req, &resp);
        if (status.ok()) {
            std::cout << "[Cliente] Create OK. ID = " << resp.id() << std::endl;
            return resp.id();
        } else {
            std::cerr << "[Cliente] Error en Create: " << status.error_message() << std::endl;
            return 0;
        }
    }

    // Escribe un valor en el bloque especificado
    void Set(uint32_t id, const std::string& value) {
        SetRequest req;
        req.set_id(id);
        req.set_value(value);
        Empty resp;
        ClientContext ctx;
        Status status = stub->Set(&ctx, req, &resp);
        if (status.ok()) {
            std::cout << "[Cliente] Set OK para ID " << id << std::endl;
        } else {
            std::cerr << "[Cliente] Error en Set: " << status.error_message() << std::endl;
        }
    }

    // Disminuye el contador de referencias de un bloque
    void DecreaseRef(uint32_t id) {
        IdRequest req;
        req.set_id(id);
        Empty resp;
        ClientContext ctx;
        Status status = stub->DecreaseRefCount(&ctx, req, &resp);
        if (status.ok()) {
            std::cout << "[Cliente] DecreaseRefCount OK para ID " << id << std::endl;
        } else {
            std::cerr << "[Cliente] Error en DecreaseRefCount: " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<MemoryManager::Stub> stub;
};

// Función principal de prueba
int main() {
    // Conectarse al servidor en localhost
    MemoryClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    std::cout << "\n== INICIO TEST DE COALESCING DE BLOQUES LIBRES ==\n" << std::endl;

    // Crear tres bloques de tipo int (tamaño 32 cada uno)
    uint32_t id1 = client.Create(32, "int");
    uint32_t id2 = client.Create(32, "int");
    uint32_t id3 = client.Create(32, "int");

    // Asignar valores
    client.Set(id1, "11");
    client.Set(id2, "22");
    client.Set(id3, "33");

    // Liberar en orden específico para activar coalescing:
    client.DecreaseRef(id2); // libera el bloque del medio
    std::this_thread::sleep_for(3s);

    client.DecreaseRef(id1); // libera el primero (adyacente al del medio)
    std::this_thread::sleep_for(3s);

    client.DecreaseRef(id3); // libera el último (adyacente también)
    std::this_thread::sleep_for(3s);

    std::cout << "\n== FIN TEST: Revisa el dump final para ver coalescing aplicado ==\n" << std::endl;

    return 0;
}
