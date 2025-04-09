#ifndef MPOINTER_MANAGER_H
#define MPOINTER_MANAGER_H

#include <memory>
#include <string>
#include <mutex>
#include <grpcpp/grpcpp.h>
#include "../generated/memory.grpc.pb.h"

class MPointerManager {
public:
    static void init(const std::string& host, int port) {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (!instance) {
            instance.reset(new MPointerManager(host, port));
        }
    }

    static MPointerManager& getInstance() {
        if (!instance) {
            throw std::runtime_error("MPointerManager no inicializado. Llamar Init primero.");
        }
        return *instance;
    }

    uint32_t create(uint32_t size, const std::string& type) {
        CreateRequest req;
        req.set_size(size);
        req.set_type(type);
        CreateResponse resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->Create(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en Create: " + status.error_message());
        return resp.id();
    }

    void set(uint32_t id, const std::string& value) {
        SetRequest req;
        req.set_id(id);
        req.set_value(value);
        Empty resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->Set(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en Set: " + status.error_message());
    }

    std::string get(uint32_t id) {
        GetRequest req;
        req.set_id(id);
        GetResponse resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->Get(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en Get: " + status.error_message());
        return resp.value();
    }

    void increaseRef(uint32_t id) {
        IdRequest req;
        req.set_id(id);
        Empty resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->IncreaseRefCount(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en IncreaseRef: " + status.error_message());
    }

    void decreaseRef(uint32_t id) {
        IdRequest req;
        req.set_id(id);
        Empty resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->DecreaseRefCount(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en DecreaseRef: " + status.error_message());
    }

private:
    MPointerManager(const std::string& host, int port) {
        std::string target = host + ":" + std::to_string(port);
        stub = MemoryManager::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
    }

    std::unique_ptr<MemoryManager::Stub> stub;
    static std::unique_ptr<MPointerManager> instance;
    static std::mutex instanceMutex;
};

inline std::unique_ptr<MPointerManager> MPointerManager::instance = nullptr;
inline std::mutex MPointerManager::instanceMutex;

#endif // MPOINTER_MANAGER_H
