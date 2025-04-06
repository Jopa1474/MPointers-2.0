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

class MemoryManagerService final : public MemoryManager::Service {
public:
    MemoryManagerService(void* memoryBase, size_t totalSize, const std::string& dumpFolder);
    ~MemoryManagerService();

    // Métodos GRPC
    grpc::Status Create(grpc::ServerContext* context,
                        const CreateRequest* request,
                        CreateResponse* response) override;

    grpc::Status Set(grpc::ServerContext* context,
                     const SetRequest* request,
                     Empty* response) override;

    grpc::Status Get(grpc::ServerContext* context,
                     const GetRequest* request,
                     GetResponse* response) override;

    grpc::Status IncreaseRefCount(grpc::ServerContext* context,
                                  const IdRequest* request,
                                  Empty* response) override;

    grpc::Status DecreaseRefCount(grpc::ServerContext* context,
                                  const IdRequest* request,
                                  Empty* response) override;

private:
    void* memoryStart;
    size_t memorySize;
    std::string dumpDirectory;

    std::mutex memMutex;
    MemoryTable table;

    // Garbage Collector
    std::thread gcThread;
    std::atomic<bool> runningGC;

    void runGarbageCollector();
};

#endif // MEMORY_MANAGER_SERVICE_H
