#include "SomeObject2.h"
#include "AllocFile.h"
using std::make_pair;
using std::pair;
using std::vector;
using std::map;
using ::allocFile;

map<void*, size_t> SomeObject2::pools;
vector<void*> SomeObject2::free;

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
        return alloc;
    } else {
        auto freeMem = free.back();
        free.pop_back();
        --pools[parentOf(freeMem)];
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
}
