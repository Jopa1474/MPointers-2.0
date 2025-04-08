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

class MemoryClient {
public:
    MemoryClient(std::shared_ptr<Channel> channel)
        : stub(MemoryManager::NewStub(channel)) {}

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
    MemoryClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    std::cout << "\n== INICIO TEST DE COALESCING DE BLOQUES LIBRES ==\n" << std::endl;

    uint32_t id1 = client.Create(32, "int");
    uint32_t id2 = client.Create(32, "int");
    uint32_t id3 = client.Create(32, "int");

    client.Set(id1, "11");
    client.Set(id2, "22");
    client.Set(id3, "33");

    client.DecreaseRef(id2); // liberar el bloque del medio primero
    std::this_thread::sleep_for(3s);

    client.DecreaseRef(id1); // liberar el primero
    std::this_thread::sleep_for(3s);

    client.DecreaseRef(id3); // liberar el tercero
    std::this_thread::sleep_for(3s);

    std::cout << "\n== FIN TEST: Revisa el dump final para ver coalescing aplicado ==\n" << std::endl;

    return 0;
}
