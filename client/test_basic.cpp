#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "../generated/memory.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class MemoryClient {
public:
    MemoryClient(std::shared_ptr<Channel> channel)
        : stub(MemoryManager::NewStub(channel)) {}

    // Solicita creación de bloque
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

    // Asigna valor al bloque
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

    // Obtiene valor del bloque
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
    std::unique_ptr<MemoryManager::Stub> stub;
};

int main() {
    MemoryClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    uint32_t id = client.Create(32, "int"); // tamaño 32 bytes, tipo int
    if (id != 0) {
        client.Set(id, "42");
        client.Get(id);
    }

    return 0;
}
