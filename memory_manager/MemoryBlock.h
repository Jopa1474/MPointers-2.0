#ifndef MEMORY_BLOCK_H
#define MEMORY_BLOCK_H

#include <string>
#include <cstddef> // size_t
#include <chrono>

// Representa un bloque individual dentro del heap reservado
struct MemoryBlock {
    uint32_t id;
    std::string type;
    size_t size;
    void* address;
    uint32_t refCount;
    std::chrono::steady_clock::time_point createdAt;

    // Constructor por defecto requerido por unordered_map
    MemoryBlock()
        : id(0), type("undefined"), size(0), address(nullptr), refCount(0),
          createdAt(std::chrono::steady_clock::now()) {}

    // Constructor principal
    MemoryBlock(uint32_t _id, const std::string& _type, size_t _size, void* _address)
        : id(_id), type(_type), size(_size), address(_address), refCount(1),
          createdAt(std::chrono::steady_clock::now()) {}
};

#endif // MEMORY_BLOCK_H
