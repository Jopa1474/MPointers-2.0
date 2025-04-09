// Esta clase se encarga de manejar la comunicación gRPC con el servidor MemoryManager.
// Es un singleton que encapsula las operaciones básicas como:
// - create: para solicitar creación de memoria remota.
// - set / get: para modificar o consultar valores remotos.
// - increaseRef / decreaseRef: para gestionar el ciclo de vida de los bloques de memoria.
//
// Es utilizada internamente por MPointer<T> para enviar las solicitudes.


#ifndef MPOINTER_MANAGER_H
#define MPOINTER_MANAGER_H

#include <memory>
#include <string>
#include <mutex>
#include <grpcpp/grpcpp.h>
#include "../generated/memory.grpc.pb.h"

class MPointerManager {
public:
    // Inicializa el singleton con la IP y puerto del servidor (host:port)
    static void init(const std::string& host, int port) {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (!instance) {
            instance.reset(new MPointerManager(host, port));
        }
    }

    // Obtiene la instancia única del singleton
    static MPointerManager& getInstance() {
        if (!instance) {
            throw std::runtime_error("MPointerManager no inicializado. Llamar Init primero.");
        }
        return *instance;
    }

    // Solicita creación de un bloque remoto con tamaño y tipo
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

    // Asigna un valor al bloque de memoria con ID dado
    void set(uint32_t id, const std::string& value) {
        SetRequest req;
        req.set_id(id);
        req.set_value(value);
        Empty resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->Set(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en Set: " + status.error_message());
    }

    // Recupera el valor del bloque de memoria con ID dado
    std::string get(uint32_t id) {
        GetRequest req;
        req.set_id(id);
        GetResponse resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->Get(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en Get: " + status.error_message());
        return resp.value();
    }

    // Incrementa el contador de referencias de un bloque (refCount++)
    void increaseRef(uint32_t id) {
        IdRequest req;
        req.set_id(id);
        Empty resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->IncreaseRefCount(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en IncreaseRef: " + status.error_message());
    }

    // Decrementa el contador de referencias de un bloque (refCount--)
    void decreaseRef(uint32_t id) {
        IdRequest req;
        req.set_id(id);
        Empty resp;
        grpc::ClientContext ctx;
        grpc::Status status = stub->DecreaseRefCount(&ctx, req, &resp);
        if (!status.ok()) throw std::runtime_error("Error en DecreaseRef: " + status.error_message());
    }

private:
    // Constructor privado: crea el stub gRPC para comunicarse con el servidor
    MPointerManager(const std::string& host, int port) {
        std::string target = host + ":" + std::to_string(port);
        stub = MemoryManager::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
    }

    // Stub gRPC generado automáticamente por Protobuf
    std::unique_ptr<MemoryManager::Stub> stub;

    // Instancia estática del singleton
    static std::unique_ptr<MPointerManager> instance;

    // Mutex para sincronizar acceso entre hilos
    static std::mutex instanceMutex;
};

// Inicialización estática de variables miembro del singleton
inline std::unique_ptr<MPointerManager> MPointerManager::instance = nullptr;
inline std::mutex MPointerManager::instanceMutex;

#endif // MPOINTER_MANAGER_H
