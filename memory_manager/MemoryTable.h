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

    uint32_t allocate(const std::string& type, size_t size) {
        std::lock_guard<std::mutex> lock(tableMutex);

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

    MemoryBlock* get(uint32_t id) {
        std::lock_guard<std::mutex> lock(tableMutex);
        auto it = blocks.find(id);
        if (it != blocks.end()) {
            return &it->second;
        }
        return nullptr;
    }

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

    const std::unordered_map<uint32_t, MemoryBlock>& getAll() const {
        return blocks;
    }

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
