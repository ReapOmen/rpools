#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <cmath>
#include <vector>

#include "tools/mallocator.h"
#include "linked_pool/GlobalLinkedPool.h"

namespace {
    using efficient_pools::GlobalLinkedPool;
    using efficient_pools::PoolHeaderG;
    using efficient_pools::NodeG;

    const size_t __threshold = 128; // malloc performs equally well
                                    // on objects of size > 128
    const size_t __mod = sizeof(void*) - 1;
    const size_t __logOfVoid = std::log2(sizeof(void*));

    std::vector<GlobalLinkedPool*,
                mallocator<std::unique_ptr<GlobalLinkedPool>>>
        __allocators(__threshold >> __logOfVoid);
}

inline size_t getAllocatorsIndex(size_t size) {
    return (size >> __logOfVoid) - 1;
}

inline void* custom_new_no_throw(size_t size) {
    // use malloc for large sizes
    if (size > __threshold) {
        return std::malloc(size);
    } else {
        size_t remainder = size & __mod; // size % sizeof(void*)
        // get the next multiple of size(void*)
        size = remainder == 0 ? size : (size + __mod) & ~__mod;
        auto poolAlloc = __allocators[getAllocatorsIndex(size)];
        if (poolAlloc) {
            // our pool was already created, just use it
            return poolAlloc->allocate();
        } else {
            // create the pool which can hold objects of size
            // <size>
            auto newPool = static_cast<GlobalLinkedPool*>(
                malloc(sizeof(GlobalLinkedPool))
            );
            new (newPool) GlobalLinkedPool(size);
            __allocators[getAllocatorsIndex(size)] = newPool;
            return newPool->allocate();
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

inline bool is_equal(const char s[8], const char s2[8]) {
    for (short i = 0; i < 7; ++i) {
        if (s[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

inline void custom_delete(void* ptr) throw() {
    const PoolHeaderG& ph = GlobalLinkedPool::getPoolHeader(ptr);
    // find out if the pointer was allocated with malloc
    // or within a pool
    if (!is_equal(ph.isPool, PoolHeaderG::IS_POOL)) {
        std::free(ptr);
    } else {
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        __allocators[getAllocatorsIndex(ph.sizeOfObjects)]
            ->deallocate(ptr);
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
