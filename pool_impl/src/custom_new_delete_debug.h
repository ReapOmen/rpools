#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <map>
#include <set>
#include <thread>
#include <iostream>
#include <algorithm>
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

    const size_t __usablePoolSize = GlobalLinkedPool::PAGE_SIZE -
                                sizeof(PoolHeaderG);

    const size_t __mallocOverhead = 8;
    const bool __useOnlyMalloc = false;

    std::map<size_t, efficient_pools::GlobalLinkedPool,
             std::less<size_t>, __Alloc> __allocators;
    // the pointers that have been allocated with malloc and
    // their respective sizes
    std::set<std::pair<void*, size_t>,
             std::less<std::pair<void*, size_t>>,
             mallocator<std::pair<void*, size_t>>> __mallocs;
    std::mutex mut;
    AllocCollector ac;
}

inline void* custom_new_no_throw(size_t size) {
    std::unique_lock<std::mutex> ul(mut);
    // use malloc for large sizes
    if (__useOnlyMalloc || size > __threshold) {
        void* toRet = std::malloc(size == 0 ? 1 : size);
        __mallocs.insert(std::make_pair(toRet, size));
        ul.unlock();
        ac.addAllocation(size);
        ac.addOverhead(__mallocOverhead);
        return toRet;
    } else {
        // sizes that are smaller than 8 are grouped together in pools
        // of size 8
        size_t newSize = size < sizeof(NodeG) ? sizeof(NodeG) : size;
        // look for a LinkedPool that can hold <newSize> objects
        auto poolAlloc = __allocators.find(newSize);
        if (poolAlloc != __allocators.end()) {
            size_t poolSize = poolAlloc->second.getNumOfPools();
            void* toRet = poolAlloc->second.allocate();
            if (poolAlloc->second.getNumOfPools() > poolSize) {
                ul.unlock();
                ac.addAllocation(__usablePoolSize);
                ac.addOverhead(sizeof(PoolHeaderG));
            }
            return toRet;
        } else {
            // a LinkedPool that can allocate objects of size
            // <newSize> was not found, so it is created and added
            // to the map
            auto newPool = GlobalLinkedPool(newSize);
            void* toRet = newPool.allocate();
            __allocators.insert(std::make_pair(newSize, std::move(newPool)));
            ul.unlock();
            ac.addAllocation(__usablePoolSize);
            ac.addOverhead(sizeof(PoolHeaderG));
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
    auto bigAlloc = std::find_if(__mallocs.begin(), __mallocs.end(),
                                 [&ptr](const std::pair<void*, size_t>& p){
                                     return (size_t*)p.first == (size_t*)ptr;
                                 });
    if (bigAlloc != __mallocs.end()) {
        __mallocs.erase(bigAlloc);
        ul.unlock();
        std::free(ptr);
        ac.removeAllocation(bigAlloc->second);
        ac.removeOverhead(__mallocOverhead);
    } else {
        // mask pointer to find out the pool header and the size of the objects
        // it can hold
        size_t size = GlobalLinkedPool::getPoolHeader(ptr).sizeOfObjects;
        auto& pool = __allocators[size];
        size_t numOfPools = pool.getNumOfPools();
        pool.deallocate(ptr);
        if (numOfPools > pool.getNumOfPools()) {
            ul.unlock();
            ac.removeAllocation(__usablePoolSize);
            ac.removeOverhead(sizeof(PoolHeaderG));
        }
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
