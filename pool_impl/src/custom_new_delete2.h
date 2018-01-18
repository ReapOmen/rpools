#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <cmath>
#include <vector>

extern "C" {
#include "avltree/avl_utils.h"
}

#include "tools/mallocator.h"
#include "tools/FreeDeleter.h"
#include "linked_pool/GlobalLinkedPool.h"
#include <iostream>
using std::cout;
using std::endl;


namespace {
    using efficient_pools::GlobalLinkedPool;
    using efficient_pools::PoolHeaderG;
    using efficient_pools::NodeG;

    const size_t __threshold = 128; // malloc performs equally well
                                    // on objects of size > 128
    const size_t __mod = sizeof(void*) - 1;
    const size_t __logOfVoid = std::log2(sizeof(void*));
    const char MARK_TYPE_POOL = (char) 0, MARK_TYPE_MALLOC = (char) 255;

    std::vector<
        std::unique_ptr<GlobalLinkedPool, FreeDeleter<GlobalLinkedPool>>,
        mallocator<std::unique_ptr<GlobalLinkedPool>>
    > __allocators(__threshold >> __logOfVoid);

    inline size_t getAllocatorsIndex(size_t size) {
        return (size >> __logOfVoid) - 1;
    }

    inline void* mark(void* addr, char markType) {
        char* cAddr = reinterpret_cast<char*>(addr);
        *cAddr = markType;
        return reinterpret_cast<void*>(cAddr + 1);
    }
}

inline void* custom_new_no_throw(size_t size) {
    // use malloc for large sizes
    if (size > __threshold) {
        return mark(std::malloc(size + 1), MARK_TYPE_MALLOC);
    } else {
        size = size == 0 ? size + 1 : size;
        size_t remainder = size & __mod; // size % sizeof(void*)
        // round up to the next multiple of <sizeof(void*)>
        // in the case where size % <sizeof(void*)> == 0, add 1
        size = remainder == 0 ? size : (size + __mod) & ~__mod;
        auto& poolAlloc = __allocators[getAllocatorsIndex(size)];
        if (poolAlloc) {
            // our pool was already created, just use it
            return mark(poolAlloc->allocate(), MARK_TYPE_POOL);
        } else {
            // create the pool which can hold objects of size <size>
            poolAlloc.reset(
                new (malloc(sizeof(GlobalLinkedPool))) GlobalLinkedPool(size + 1)
            );
            return mark(poolAlloc->allocate(), MARK_TYPE_POOL);
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
    // find out if the pointer was allocated with malloc
    // or within a pool
    char* addr = reinterpret_cast<char*>(ptr);
    addr--;
    if (*addr == MARK_TYPE_MALLOC) {
        std::free(addr);
    } else {
        const PoolHeaderG& ph = GlobalLinkedPool::getPoolHeader(addr);
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        __allocators[getAllocatorsIndex(ph.sizeOfObjects)]
            ->deallocate(addr);
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
