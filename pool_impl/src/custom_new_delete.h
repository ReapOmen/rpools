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

inline void* custom_new(size_t size) throw(std::bad_alloc) {
    // if using a LinkedPool is too inneficient we will use malloc
    static AllocCollector ac;
    ac.addAllocation(size);
    void* toRet = nullptr;
    if (size > __threshold) {
        toRet = std::malloc(size);
        __mallocs.insert(toRet);
    } else {
        size = size < sizeof(NodeG) ? sizeof(NodeG) : size;
        auto poolAlloc = __allocators.find(size);
        if (poolAlloc != __allocators.end()) {
            toRet = poolAlloc->second.allocate();
        } else {
            auto newPool = GlobalLinkedPool(size);
            toRet = newPool.allocate();
            __allocators.insert(std::make_pair(size, std::move(newPool)));
        }
    }
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

inline void custom_delete(void* ptr) throw() {
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
