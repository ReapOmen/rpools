#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <map>
#include <set>

#include "linked_pool/GlobalLinkedPool.h"
#include "CustomAlloc.h"

using __Alloc = mallocator<std::pair<const size_t,
                                     efficient_pools::GlobalLinkedPool>>;

std::map<size_t, efficient_pools::GlobalLinkedPool,
         std::less<size_t>, __Alloc> __allocators;
std::set<void*, std::less<void*>, mallocator<void*>> __mallocs;

void* operator new(size_t size) {
    using efficient_pools::GlobalLinkedPool;
    using efficient_pools::PoolHeader;
    using efficient_pools::Node;

    // if using a LinkedPool is too inneficient we will use malloc
    if (size < sizeof(Node) ||
        size > (GlobalLinkedPool::PAGE_SIZE - sizeof(PoolHeader)) / 4) {
        void* memLocation = std::malloc(size);
        __mallocs.insert(memLocation);
        return memLocation;
    } else {
        auto poolAlloc = __allocators.find(size);
        if (poolAlloc != __allocators.end()) {
            return poolAlloc->second.allocate();
        } else {
            auto newPool = GlobalLinkedPool(size);
            __allocators.insert(std::make_pair(size, newPool));
            return newPool.allocate();
        }
    }
}

void operator delete(void* ptr) noexcept {
    using efficient_pools::GlobalLinkedPool;

    auto bigAlloc = __mallocs.find(ptr);
    if (bigAlloc != __mallocs.end()) {
        __mallocs.erase(bigAlloc);
        std::free(ptr);
    } else {
        size_t size = GlobalLinkedPool::getPoolHeader(ptr).sizeOfObjects;
        __allocators[size].deallocate(ptr);
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
