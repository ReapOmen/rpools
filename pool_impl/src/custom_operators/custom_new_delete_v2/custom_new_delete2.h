#ifndef __CUSTOM_NEW_DELETE2_H__
#define __CUSTOM_NEW_DELETE2_H__

#include <cmath>
#include <vector>

#include "tools/mallocator.h"
#include "tools/FreeDeleter.h"
#include "pool_allocators/GlobalLinkedPool.h"

namespace {
    using efficient_pools::GlobalLinkedPool;

    const size_t __threshold = 128; // malloc performs equally well
                                    // on objects of size > 128
    const size_t __mod = sizeof(void*) - 1;
    const size_t __logOfVoid = std::log2(sizeof(void*));
    const char MARK_TYPE_POOL = (char) 0, MARK_TYPE_MALLOC = (char) 255;

    std::vector<
        std::unique_ptr<GlobalLinkedPool, FreeDeleter<GlobalLinkedPool>>,
        mallocator<std::unique_ptr<GlobalLinkedPool>>
    > __allocators(__threshold >> __logOfVoid);

    inline size_t getAllocatorsIndex(size_t t_size) {
        return (t_size >> __logOfVoid) - 1;
    }

    inline void* mark(void* t_addr, char t_markType) {
        char* cAddr = reinterpret_cast<char*>(t_addr);
        *cAddr = t_markType;
        return reinterpret_cast<void*>(cAddr + 1);
    }
}

inline void* custom_new_no_throw(size_t t_size) {
    // use malloc for large sizes
    if (t_size > __threshold) {
        return mark(std::malloc(t_size + 1), MARK_TYPE_MALLOC);
    } else {
        t_size = t_size == 0 ? t_size + 1 : t_size;
        size_t remainder = t_size & __mod; // size % sizeof(void*)
        // round up to the next multiple of <sizeof(void*)>
        // in the case where size % <sizeof(void*)> == 0, add 1
        t_size = remainder == 0 ? t_size : (t_size + __mod) & ~__mod;
        auto& poolAlloc = __allocators[getAllocatorsIndex(t_size)];
        if (poolAlloc) {
            // our pool was already created, just use it
            return mark(poolAlloc->allocate(), MARK_TYPE_POOL);
        } else {
            // create the pool which can hold objects of size <size>
            poolAlloc.reset(
                new (malloc(sizeof(GlobalLinkedPool)))
                    GlobalLinkedPool(t_size + 1)
            );
            return mark(poolAlloc->allocate(), MARK_TYPE_POOL);
        }
    }
}

inline void* custom_new(size_t t_size) {
    void* toRet = custom_new_no_throw(t_size);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

inline void custom_delete(void* t_ptr) throw() {
    // find out if the pointer was allocated with malloc
    // or within a pool
    char* addr = reinterpret_cast<char*>(t_ptr);
    addr--;
    if (*addr == MARK_TYPE_MALLOC) {
        std::free(addr);
    } else {
        const PoolHeaderG& ph = GlobalLinkedPool::getPoolHeader(addr);
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        __allocators[getAllocatorsIndex(ph.sizeOfSlot - 1)]
            ->deallocate(addr);
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
