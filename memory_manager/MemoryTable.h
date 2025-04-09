// Clase responsable de administrar la tabla de bloques de memoria del sistema.
// Lleva el control de todos los bloques asignados, su metadata y la Free List
// (huecos disponibles). Es utilizada exclusivamente por el MemoryManagerService.
//
// Implementa:
//  - asignación de memoria (con reutilización de huecos libres)
//  - manejo de refcount
//  - eliminación y liberación de bloques
//  - fusión de huecos contiguos (coalescing)
//  - obtención de información de diagnóstico para los dumps

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
    // Constructor que recibe el bloque base de memoria y su tamaño total
    MemoryTable(void* baseAddress, size_t totalSize)
        : base(baseAddress), maxSize(totalSize), currentOffset(0), nextId(1) {}

    // Solicita un bloque de memoria del tipo y tamaño especificado.
    // Primero intenta reutilizar huecos de la freeList. Si no hay, lo asigna secuencialmente.
    uint32_t allocate(const std::string& type, size_t size) {
        std::lock_guard<std::mutex> lock(tableMutex);

        // Buscar hueco disponible (first-fit)
        for (size_t i = 0; i < freeList.size(); ++i) {
            auto& [offset, freeSize] = freeList[i];
            if (freeSize >= size) {
                void* address = static_cast<char*>(base) + offset;
                uint32_t id = nextId++;

                MemoryBlock block(id, type, size, address);
                blocks[id] = block;

                // Actualizar hueco libre (consumido total o parcialmente)
                if (freeSize == size) {
                    freeList.erase(freeList.begin() + i);
                } else {
                    freeList[i].first += size;
                    freeList[i].second -= size;
                }

                return id;
            }
        }

        // Si no se encontró hueco, intentar asignar al final
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

    // Devuelve un puntero al bloque con el ID dado, o nullptr si no existe
    MemoryBlock* get(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Incrementa el contador de referencias de un bloque
    void increaseRef(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end()) {
            it->second.refCount++;
        }
    }

    // Decrementa el contador de referencias de un bloque
    void decreaseRef(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end() && it->second.refCount > 0) {
            it->second.refCount--;
        }
    }

    // Retorna todos los bloques actuales (por referencia constante)
    const std::unordered_map<uint32_t, MemoryBlock>& getAll() const {
        return blocks;
    }

    // Elimina un bloque y lo libera, agregando su espacio a la Free List
    // Aplica coalescing automáticamente si hay bloques contiguos
    void remove(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end()) {
            size_t offset = static_cast<char*>(it->second.address) - static_cast<char*>(base);
            size_t size = it->second.size;
            freeList.emplace_back(offset, size);
            blocks.erase(it);
            coalesceFreeList();
        }
    }

    // Devuelve la lista de huecos libres (offset, tamaño)
    std::vector<std::pair<size_t, size_t>> getFreeList() const {
        return freeList;
    }

private:
    void* base; // Dirección base del bloque reservado (malloc)
    size_t maxSize; // Tamaño total del bloque
    size_t currentOffset; // Offset actual donde termina el uso secuencial
    uint32_t nextId; // ID incremental para cada nuevo bloque

    std::unordered_map<uint32_t, MemoryBlock> blocks; // Bloques activos
    std::vector<std::pair<size_t, size_t>> freeList; // Lista de huecos libres
    std::mutex tableMutex; // Mutex para acceso concurrente seguro

    // Fusiona bloques contiguos en la Free List para reducir fragmentación externa
    void coalesceFreeList() {
        if (freeList.size() <= 1) return;

        std::sort(freeList.begin(), freeList.end());

        std::vector<std::pair<size_t, size_t>> merged;
        merged.push_back(freeList[0]);

        for (size_t i = 1; i < freeList.size(); ++i) {
            auto& last = merged.back();
            size_t lastEnd = last.first + last.second;
            size_t currentStart = freeList[i].first;
            size_t currentSize = freeList[i].second;

            if (lastEnd == currentStart) {
                last.second += currentSize;
            } else {
                merged.push_back(freeList[i]);
            }
        }

        freeList = std::move(merged);
    }
};

#endif // MEMORY_TABLE_H
