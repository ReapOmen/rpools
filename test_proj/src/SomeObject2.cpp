#include "SomeObject2.h"
#include "AllocFile.h"
using std::make_pair;
using std::pair;
using std::vector;
using std::map;
using std::max;

map<void*, size_t> SomeObject2::pools;
vector<void*> SomeObject2::free;
size_t SomeObject2::overhead = 0;

size_t calculateOverheadSize() {
    size_t freeSize = 16 + sizeof(SomeObject2::free) +
        sizeof(void*) * SomeObject2::free.capacity();
    size_t poolsSize = 16 + sizeof(SomeObject2::pools) +
        (16 + sizeof(pair<void*, size_t>)) * SomeObject2::pools.size();
    return freeSize + poolsSize;
}

void* parentOf(void* ptr) {
    auto itr = SomeObject2::pools.find(ptr);
    return itr != SomeObject2::pools.end() ?
        (void*)((char*)ptr - sizeof(SomeObject2)) : ptr;
}

void* SomeObject2::operator new(size_t size) {
    if (free.empty()) {
        void* alloc = std::malloc(size << 1);
        allocFile.processAllocation(size << 1);
        pools.insert(make_pair(alloc, 1));
        void* ptr = (void*)((SomeObject2*)alloc + 1);
        free.push_back(ptr);
        overhead = max(overhead, calculateOverheadSize());
        return alloc;
    } else {
        auto freeMem = free.back();
        free.pop_back();
        --pools[parentOf(freeMem)];
        overhead = max(overhead, calculateOverheadSize());
        return ::new(freeMem) SomeObject2();
    }
}

void SomeObject2::operator delete(void *ptr) {
    reinterpret_cast<SomeObject2*>(ptr)->~SomeObject2();
    void* parent = parentOf(ptr);
    size_t newSize = ++pools[parent];
    if (newSize == 2) {
        ::allocFile.processDeallocation(sizeof(SomeObject2) << 1);
        std::free(parent);
    } else {
        free.push_back(ptr);
    }
    overhead = max(overhead, calculateOverheadSize());
}
