#include "SomeObject3.h"
#include "AllocFile.h"
using std::make_pair;
using std::pair;
using std::vector;
using std::map;
using std::max;

size_t SomeObject3::POOL_SIZE = 100;
map<void*, size_t> SomeObject3::pools;
vector<void*> SomeObject3::free;
size_t SomeObject3::overhead = 0;

size_t calculateOverheadSize2() {
    size_t freeSize = 16 + sizeof(SomeObject3::free) +
        sizeof(void*) * SomeObject3::free.capacity();
    size_t poolsSize = 16 + sizeof(SomeObject3::pools) +
        (16 + sizeof(pair<void*, size_t>)) * SomeObject3::pools.size();
    return freeSize + poolsSize;
}

void* parentOf2(void* ptr) {
    for (const auto& pair : SomeObject3::pools) {
        auto somePtr = (SomeObject3*)pair.first;
        if (ptr >= somePtr &&
            ptr < somePtr + (SomeObject3::POOL_SIZE)) {
            return pair.first;
        }
    }
}

void* SomeObject3::operator new(size_t size) {
    if (free.empty()) {
        void* alloc = std::malloc(size * POOL_SIZE);
        allocFile.processAllocation(size * POOL_SIZE);
        pools.insert(make_pair(alloc, 1));
        void* ptr = NULL;
        for (int i = 1; i < POOL_SIZE; ++i) {
            ptr = (void*)((SomeObject3*)alloc + i);
            free.push_back(ptr);
        }
        overhead = max(overhead, calculateOverheadSize2());
        return alloc;
    } else {
        auto freeMem = free.back();
        free.pop_back();
        --pools[parentOf2(freeMem)];
        overhead = max(overhead, calculateOverheadSize2());
        return ::new(freeMem) SomeObject3();
    }
}

void SomeObject3::operator delete(void *ptr) {
    reinterpret_cast<SomeObject3*>(ptr)->~SomeObject3();
    void* parent = parentOf2(ptr);
    size_t newSize = ++pools[parent];
    if (newSize == POOL_SIZE) {
        ::allocFile.processDeallocation(sizeof(SomeObject3) * POOL_SIZE);
        std::free(parent);
    } else {
        free.push_back(ptr);
    }
    overhead = max(overhead, calculateOverheadSize2());
}
