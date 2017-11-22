#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <map>
#include <set>
#include <thread>
#include "CustomAlloc.h"
#include "AllocCollector.h"
#include "linked_pool/GlobalLinkedPool.h"

namespace {
    using efficient_pools::GlobalLinkedPool;
    using efficient_pools::PoolHeaderG;
    using efficient_pools::NodeG;
    using __Alloc = mallocator<std::pair<const size_t,
                                         efficient_pools::GlobalLinkedPool>>;

    const size_t __threshold = (GlobalLinkedPool::PAGE_SIZE -
                                sizeof(PoolHeaderG)) / 4;

    std::map<size_t, efficient_pools::GlobalLinkedPool,
             std::less<size_t>, __Alloc> __allocators;
    std::set<void*, std::less<void*>, mallocator<void*>> __mallocs;
}

void* operator new(size_t size) {
    // if using a LinkedPool is too inneficient we will use malloc
    static AllocCollector ac;
    ac.addAllocation(size);
    if (size > __threshold) {
        void* memLocation = std::malloc(size);
        __mallocs.insert(memLocation);
        return memLocation;
    } else {
        size = size < sizeof(NodeG) ? sizeof(NodeG) : size;
        auto poolAlloc = __allocators.find(size);
        if (poolAlloc != __allocators.end()) {
            return poolAlloc->second.allocate();
        } else {
            auto newPool = GlobalLinkedPool(size);
            void* toReturn = newPool.allocate();
            __allocators.insert(std::make_pair(size, std::move(newPool)));
            return toReturn;
        }
    }
}

void operator delete(void* ptr) noexcept {
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
