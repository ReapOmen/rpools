#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <map>
#include <set>
#include <thread>
#include <mutex>
#include "CustomAlloc.h"
#include "AllocCollector.h"
#include "linked_pool/GlobalLinkedPool.h"

namespace {
    using efficient_pools::GlobalLinkedPool;
    using efficient_pools::PoolHeaderG;
    using efficient_pools::NodeG;
    // shorthand for allocator that can create pairs of size
    // and LinkedPools
    using __Alloc = mallocator<std::pair<const size_t,
                                         efficient_pools::GlobalLinkedPool>>;

    const size_t __threshold = (GlobalLinkedPool::PAGE_SIZE -
                                sizeof(PoolHeaderG)) / 4;

    std::map<size_t, efficient_pools::GlobalLinkedPool,
             std::less<size_t>, __Alloc> __allocators;
    // the pointers that have been allocated with malloc
    std::set<void*, std::less<void*>, mallocator<void*>> __mallocs;
    std::mutex mut;
}

inline void* custom_new_no_throw(size_t size) {
    static AllocCollector ac;
    ac.addAllocation(size);
    std::unique_lock<std::mutex> ul(mut);
    // use malloc for large sizes
    if (size > __threshold) {
        void* toRet = std::malloc(size == 0 ? 1 : size);
        __mallocs.insert(toRet);
        return toRet;
    } else {
        // sizes that are smaller than 8 are grouped together in pools
        // of size 8
        size_t newSize = size < sizeof(NodeG) ? sizeof(NodeG) : size;
        // look for a LinkedPool that can hold <newSize> objects
        auto poolAlloc = __allocators.find(newSize);
        if (poolAlloc != __allocators.end()) {
            return poolAlloc->second.allocate();
        } else {
            // a LinkedPool that can allocate objects of size
            // <newSize> was not found, so it is created and added
            // to the map
            auto newPool = GlobalLinkedPool(newSize);
            void* toRet = newPool.allocate();
            __allocators.insert(std::make_pair(newSize, std::move(newPool)));
            return toRet;
        }
    }
}

inline void* custom_new(size_t size) {
    void* toRet = custom_new_no_throw(size);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

inline void custom_delete(void* ptr) throw() {
    std::unique_lock<std::mutex> ul(mut);
    // find out if the pointer was allocated with malloc
    auto bigAlloc = __mallocs.find(ptr);
    if (bigAlloc != __mallocs.end()) {
        __mallocs.erase(bigAlloc);
        std::free(ptr);
    } else {
        // mask pointer to find out the pool header and the size of the objects
        // it can hold
        size_t size = GlobalLinkedPool::getPoolHeader(ptr).sizeOfObjects;
        __allocators[size].deallocate(ptr);
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
