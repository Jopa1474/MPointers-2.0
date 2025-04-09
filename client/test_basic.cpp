// Este archivo contiene una prueba básica para validar que el servidor de
// Memory Manager puede crear bloques de memoria, asignarles un valor y luego
// recuperar dicho valor usando gRPC directamente sin usar la clase MPointer.
//
// Esta prueba se conecta al servidor en localhost:50051 y opera sobre un bloque
// de tipo int y tamaño 32 bytes.

#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "../generated/memory.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Clase que representa el cliente gRPC para comunicarse con el Memory Manager
class MemoryClient {
public:
    // Constructor: recibe un canal gRPC para establecer la conexión con el servidor
    MemoryClient(std::shared_ptr<Channel> channel)
        : stub(MemoryManager::NewStub(channel)) {}

    // Solicita al servidor crear un nuevo bloque de memoria
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

    // Solicita al servidor escribir un valor en un bloque de memoria
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

    // Solicita al servidor recuperar el valor almacenado en un bloque
    void Get(uint32_t id) {
        GetRequest req;
        req.set_id(id);

        GetResponse resp;
        ClientContext ctx;
        Status status = stub->Get(&ctx, req, &resp);

        if (status.ok()) {
            std::cout << "[Cliente] Valor leído de ID " << id << ": " << resp.value() << std::endl;
        } else {
            std::cerr << "[Cliente] Error en Get: " << status.error_message() << std::endl;
        }
    }

private:
    // Stub generado automáticamente por gRPC para enviar peticiones al servidor
    std::unique_ptr<MemoryManager::Stub> stub;
};

// Punto de entrada de la aplicación de prueba
int main() {
    // Crear cliente conectado a localhost en el puerto 50051
    MemoryClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    // Crear bloque de tipo int (32 bytes) y operar sobre él
    uint32_t id = client.Create(32, "int");
    if (id != 0) {
        client.Set(id, "42");    // Escribe el valor 42
        client.Get(id);          // Lee y muestra el valor
    }

    return 0;
}
