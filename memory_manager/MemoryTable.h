#ifndef MEMORY_TABLE_H
#define MEMORY_TABLE_H

#include "MemoryBlock.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

class MemoryTable {
public:
    MemoryTable(void* baseAddress, size_t totalSize)
        : base(baseAddress), maxSize(totalSize), currentOffset(0), nextId(1) {}

    // Crear bloque reutilizando huecos si es posible
    uint32_t allocate(const std::string& type, size_t size) {
        std::lock_guard<std::mutex> lock(tableMutex);

        // Buscar hueco disponible (First Fit)
        for (size_t i = 0; i < freeList.size(); ++i) {
            auto& [offset, freeSize] = freeList[i];
            if (freeSize >= size) {
                void* address = static_cast<char*>(base) + offset;
                uint32_t id = nextId++;

                MemoryBlock block(id, type, size, address);
                blocks[id] = block;

                if (freeSize == size) {
                    freeList.erase(freeList.begin() + i);
                } else {
                    freeList[i].first += size;
                    freeList[i].second -= size;
                }

                return id;
            }
        }

        // Si no hay hueco suficiente, continuar desde el offset
        if (currentOffset + size > maxSize) {
            throw std::runtime_error("No hay suficiente memoria disponible.");
        }

        void* address = static_cast<char*>(base) + currentOffset;
        uint32_t id = nextId++;

        MemoryBlock block(id, type, size, address);
        blocks[id] = block;
        currentOffset += size;

        return id;
    }

    // Obtener bloque por ID
    MemoryBlock* get(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Incrementar/decrementar referencias
    void increaseRef(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end()) {
            it->second.refCount++;
        }
    }

    void decreaseRef(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end() && it->second.refCount > 0) {
            it->second.refCount--;
        }
    }

    // Obtener todos los bloques activos
    const std::unordered_map<uint32_t, MemoryBlock>& getAll() const {
        return blocks;
    }

    // Eliminar un bloque y registrar su espacio en la free list
    void remove(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end()) {
            size_t offset = static_cast<char*>(it->second.address) - static_cast<char*>(base);
            size_t size = it->second.size;
            freeList.emplace_back(offset, size);
            blocks.erase(it);
        }
    }

    // Exponer la free list para visualización en dumps
    std::vector<std::pair<size_t, size_t>> getFreeList() const {
        return freeList;
    }

private:
    void* base;
    size_t maxSize;
    size_t currentOffset;
    uint32_t nextId;

    std::unordered_map<uint32_t, MemoryBlock> blocks;
    std::vector<std::pair<size_t, size_t>> freeList;
    std::mutex tableMutex;
};

#endif // MEMORY_TABLE_H
