// Esta prueba verifica el funcionamiento correcto del Garbage Collector.
// Se crea un bloque de memoria, se le asigna un valor, se reduce su refcount a 0
// y luego se espera un tiempo para que el GC lo libere.
// Finalmente, se intenta hacer un Get que debería fallar si fue eliminado correctamente.

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

// Cliente gRPC que se comunica con el servidor MemoryManager
class MemoryClient {
public:
    explicit MemoryClient(std::shared_ptr<Channel> channel)
        : stub(MemoryManager::NewStub(channel)) {}

    // Solicita la creación de un bloque de memoria
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

    // Escribe un valor en el bloque
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

    // Lee el valor almacenado en el bloque
    void Get(uint32_t id) {
        GetRequest req;
        req.set_id(id);
        GetResponse resp;
        ClientContext ctx;
        Status status = stub->Get(&ctx, req, &resp);
        if (status.ok()) {
            std::cout << "[Cliente] Get OK. Valor = " << resp.value() << std::endl;
        } else {
            std::cerr << "[Cliente] Error en Get: " << status.error_message() << std::endl;
        }
    }

    // Disminuye el refcount del bloque (simula eliminación de referencia)
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

int main() {
    // Se conecta al servidor gRPC local
    MemoryClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    std::cout << "\n== INICIANDO TEST DEL GARBAGE COLLECTOR ==\n" << std::endl;

    // Paso 1: crear bloque y asignar valor
    uint32_t id = client.Create(32, "int");
    client.Set(id, "99");
    client.Get(id);

    // Paso 2: simular destrucción de MPointer reduciendo refcount
    client.DecreaseRef(id);

    std::cout << "[Cliente] Esperando 4 segundos para que el GC actúe..." << std::endl;
    std::this_thread::sleep_for(4s);

    // Paso 3: intentar acceder nuevamente (debería fallar)
    std::cout << "[Cliente] Intentando Get nuevamente (debería fallar):" << std::endl;
    client.Get(id);

    return 0;
}
