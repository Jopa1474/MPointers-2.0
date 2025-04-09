// Implementación del servicio gRPC MemoryManager. Este archivo contiene toda
// la lógica para gestionar la memoria remota: creación de bloques, lectura,
// escritura, manejo de referencias, recolección de basura y generación de dumps.


#include "MemoryManagerService.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <cstring>
#include <filesystem>

using namespace std;

// Función auxiliar para obtener timestamp en formato legible
static std::string obtenerTimestamp() {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto in_time_t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S")
       << "-" << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

//  Función que genera un archivo dump con:
// - Bloques en uso
// - Huecos en la Free List
// - Estadísticas globales
void crearDump(const MemoryTable& table, const std::string& path, size_t totalMemBytes) {
    std::ofstream out(path);
    if (!out.is_open()) return;

    size_t memoriaUsada = 0;

    out << "==== DUMP DE MEMORIA ====\n";
    for (const auto& [id, block] : table.getAll()) {
        memoriaUsada += block.size;

        out << "ID: " << id
            << " | Tipo: " << block.type
            << " | Size: " << block.size
            << " | RefCount: " << block.refCount
            << " | Dirección: " << block.address;

        // Mostrar valor en memoria (si es tipo soportado)
        if (block.type == "int") {
            out << " | Valor: " << *static_cast<int*>(block.address);
        } else if (block.type == "uint32_t") {
            out << " | Valor: " << *static_cast<uint32_t*>(block.address);
        } else if (block.type == "float") {
            out << " | Valor: " << *static_cast<float*>(block.address);
        } else if (block.type == "double") {
            out << " | Valor: " << *static_cast<double*>(block.address);
        } else if (block.type == "string") {
            out << " | Valor: " << static_cast<char*>(block.address);
        }

        out << "\n";
    }

    out << "\n==== HUECOS DISPONIBLES (Free List) ====\n";
    auto freeList = table.getFreeList();
    size_t memoriaLibre = 0;

    if (freeList.empty()) {
        out << "Sin huecos actualmente.\n";
    } else {
        for (const auto& [offset, size] : freeList) {
            memoriaLibre += size;
            out << "Offset: " << offset << " | Size: " << size << "\n";
        }
    }

    out << "\n==== ESTADÍSTICAS DE MEMORIA ====\n";
    out << "Total reservado: " << totalMemBytes << " bytes\n";
    out << "Memoria usada : " << memoriaUsada << " bytes\n";
    out << "Memoria libre : " << memoriaLibre << " bytes\n";
    out << "Uso (%)       : " << (100.0 * memoriaUsada / totalMemBytes) << "%\n";

    out.close();
}

//  Constructor del servicio: inicializa tabla y lanza el GC
MemoryManagerService::MemoryManagerService(void* memoryBase, size_t totalSize, const std::string& dumpFolder)
    : memoryStart(memoryBase), memorySize(totalSize), dumpDirectory(dumpFolder), table(memoryBase, totalSize) {
    std::cout << "MemoryManagerService iniciado." << std::endl;

    runningGC = true;
    gcThread = std::thread(&MemoryManagerService::runGarbageCollector, this);
}

//  Destructor → detiene GC
MemoryManagerService::~MemoryManagerService() {
    runningGC = false;
    if (gcThread.joinable()) {
        gcThread.join();
    }
    std::cout << "MemoryManagerService finalizado." << std::endl;
}

//  gRPC: crear nuevo bloque en memoria
grpc::Status MemoryManagerService::Create(grpc::ServerContext*, const CreateRequest* request, CreateResponse* response) {
    const std::string& tipo = request->type();
    uint32_t tam = request->size();

    try {
        uint32_t id = table.allocate(tipo, tam);
        response->set_id(id);
        std::cout << "[Create] Bloque creado con ID = " << id << ", tipo = " << tipo << ", tamaño = " << tam << std::endl;
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, e.what());
    }

    return grpc::Status::OK;
}

//  gRPC: escribir valor en memoria remota
grpc::Status MemoryManagerService::Set(grpc::ServerContext*, const SetRequest* request, Empty*) {
    std::lock_guard<std::mutex> lock(memMutex);

    uint32_t id = request->id();
    const std::string& value = request->value();

    MemoryBlock* block = table.get(id);
    if (!block) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "ID no encontrado");
    }

    try {
        if (block->type == "int") {
            int val = std::stoi(value);
            std::memcpy(block->address, &val, sizeof(int));
        } else if (block->type == "uint32_t") {
            uint32_t val = static_cast<uint32_t>(std::stoul(value));
            std::memcpy(block->address, &val, sizeof(uint32_t));
        } else if (block->type == "float") {
            float val = std::stof(value);
            std::memcpy(block->address, &val, sizeof(float));
        } else if (block->type == "double") {
            double val = std::stod(value);
            std::memcpy(block->address, &val, sizeof(double));
        } else if (block->type == "string") {
            std::strncpy(static_cast<char*>(block->address), value.c_str(), block->size);
        } else {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Tipo no soportado");
        }
    } catch (...) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Error convirtiendo valor");
    }

    std::filesystem::create_directories(dumpDirectory);
    std::string filename = dumpDirectory + "/dump_" + obtenerTimestamp() + ".txt";
    crearDump(table, filename, memorySize);

    std::cout << "[Set] Valor escrito en ID = " << id << std::endl;
    return grpc::Status::OK;
}

//  gRPC: leer valor remoto
grpc::Status MemoryManagerService::Get(grpc::ServerContext*, const GetRequest* request, GetResponse* response) {
    std::lock_guard<std::mutex> lock(memMutex);

    uint32_t id = request->id();
    MemoryBlock* block = table.get(id);
    if (!block) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "ID no encontrado");
    }

    std::ostringstream out;

    if (block->type == "int") {
        out << *static_cast<int*>(block->address);
    } else if (block->type == "uint32_t") {
        out << *static_cast<uint32_t*>(block->address);
    } else if (block->type == "float") {
        out << *static_cast<float*>(block->address);
    } else if (block->type == "double") {
        out << *static_cast<double*>(block->address);
    } else if (block->type == "string") {
        out << static_cast<char*>(block->address);
    } else {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Tipo no soportado");
    }

    response->set_value(out.str());
    std::cout << "[Get] Valor leído de ID = " << id << std::endl;
    return grpc::Status::OK;
}

//  gRPC: aumentar refCount
grpc::Status MemoryManagerService::IncreaseRefCount(grpc::ServerContext*, const IdRequest* request, Empty*) {
    table.increaseRef(request->id());
    std::cout << "[IncreaseRefCount] ID = " << request->id() << std::endl;
    return grpc::Status::OK;
}

//  gRPC: disminuir refCount
grpc::Status MemoryManagerService::DecreaseRefCount(grpc::ServerContext*, const IdRequest* request, Empty*) {
    table.decreaseRef(request->id());
    std::cout << "[DecreaseRefCount] ID = " << request->id() << std::endl;
    return grpc::Status::OK;
}

//  Hilo de Garbage Collector → ejecuta periódicamente
void MemoryManagerService::runGarbageCollector() {
    using namespace std::chrono_literals;

    while (runningGC) {
        std::this_thread::sleep_for(2s);

        std::lock_guard<std::mutex> lock(memMutex);
        auto all = table.getAll();

        std::vector<uint32_t> toRemove;
        for (const auto& [id, block] : all) {
            if (block.refCount == 0) {
                toRemove.push_back(id);
                std::cout << "[GC] Liberando ID = " << id << std::endl;
            }
        }

        for (uint32_t id : toRemove) {
            table.remove(id);
        }

        if (!toRemove.empty()) {
            std::string filename = dumpDirectory + "/gc_dump_" + obtenerTimestamp() + ".txt";
            crearDump(table, filename, memorySize);
        }
    }
}
