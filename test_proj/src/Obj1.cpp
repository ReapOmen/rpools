#include "Obj1.h"
#include "AllocFile.h"

#include <algorithm>
using std::make_pair;
using std::pair;
using std::vector;
using std::map;
using std::max;

size_t Obj1::POOL_SIZE = 80;
size_t Obj1::METADATA_SIZE = Obj1::POOL_SIZE / 8;
size_t Obj1::overhead = 0;
vector<void*> Obj1::pools;

size_t Obj1::calculateOverhead() {
    return 16 + METADATA_SIZE + sizeof(vector<void*>) + pools.size() * sizeof(void*);
}

pair<void*, size_t> Obj1::getPool(void* ptr) {
    for (const auto& pool : pools) {
        char* poolByte = (char*) pool;
        if (ptr > pool &&
                ptr < poolByte + METADATA_SIZE  + sizeof(Obj1) * POOL_SIZE) {
            return make_pair(
                pool,
                (Obj1*) ptr - (Obj1*)(poolByte + METADATA_SIZE)
            );
        }
    }
    return make_pair(nullptr, 0);
}

size_t Obj1::getFreeCount(void* ptr) {
    size_t count = 0;
    for (size_t i = 0; i < METADATA_SIZE; ++i) {
        char* metadata = (char*) ptr + i;
        for (int i = 7; i >= 0; --i) {
            if (!((*metadata >> i) & 0x01)) {
                ++count;
            }
        }
    }
    return count;
}

void* Obj1::nextFree(void* ptr) {
    for (size_t b = 0; b < METADATA_SIZE; ++b) {
        char* metadata = (char*) ptr + b;
        for (int i = 7; i >= 0; --i) {
            if (!((*metadata >> i) & 0x01)) {
                *metadata |= 1 << i;
                void* firstObj = (char*) ptr + METADATA_SIZE;
                return (Obj1*)firstObj + (7 - i) + b * 8;
            }
        }
    }
    return nullptr;
}

void* Obj1::operator new(size_t size) {
    for (const auto& pool : pools) {
        void* free = nextFree(pool);
        if (free != nullptr) {
            return free;
        }
    }
    void* pool = std::malloc(METADATA_SIZE  + size * POOL_SIZE);
    for (size_t b = 0; b < METADATA_SIZE; ++b) {
        char* metadata = (char*) pool + b;
        *metadata = 0;
    }
    pools.push_back(pool);
    overhead = std::max(calculateOverhead(), overhead);
    return nextFree(pool);
}

void Obj1::operator delete(void* ptr) {
    reinterpret_cast<Obj1*>(ptr)->~Obj1();
    auto pair = getPool(ptr);
    size_t metadataOffset = pair.second / 8;
    char* metadata = (char*)pair.first + metadataOffset;
    *metadata ^= 1 << (7 - pair.second % 8);
    if (getFreeCount(pair.first) == POOL_SIZE) {
        std::free(pair.first);
        pools.erase(std::find(pools.begin(), pools.end(), pair.first));
    }
    overhead = std::max(calculateOverhead(), overhead);
}
