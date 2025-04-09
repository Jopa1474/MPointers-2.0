// Este archivo define la estructura MemoryBlock, que representa cada bloque de memoria
// gestionado por el Memory Manager. Contiene toda la información necesaria para
// identificar, ubicar y administrar un bloque dentro del heap reservado.

#ifndef MEMORY_BLOCK_H
#define MEMORY_BLOCK_H

#include <string>
#include <cstddef> // size_t
#include <chrono>

// Estructura que representa un bloque de memoria dentro del heap reservado.
struct MemoryBlock {
    uint32_t id;        // ID único asignado por MemoryTable
    std::string type;   // Tipo de dato ("int", "float", "string", etc.)
    size_t size;        // Tamaño en bytes del bloque
    void* address;      // Dirección (relativa al malloc principal)
    uint32_t refCount;  // Cantidad de referencias activas (para GC)
    std::chrono::steady_clock::time_point createdAt; // Momento de creación (opcional)

    // Constructor por defecto requerido por std::unordered_map
    MemoryBlock()
        : id(0), type("undefined"), size(0), address(nullptr), refCount(0),
          createdAt(std::chrono::steady_clock::now()) {}

    // Constructor principal usado en la asignación de bloques
    MemoryBlock(uint32_t _id, const std::string& _type, size_t _size, void* _address)
        : id(_id), type(_type), size(_size), address(_address), refCount(1),
          createdAt(std::chrono::steady_clock::now()) {}
};

#endif // MEMORY_BLOCK_H
