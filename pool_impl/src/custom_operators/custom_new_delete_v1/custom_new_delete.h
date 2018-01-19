#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <cmath>
#include <vector>

#include "tools/mallocator.h"
#include "tools/FreeDeleter.h"
#include "pool_allocators/GlobalLinkedPool.h"

namespace {
    using efficient_pools::GlobalLinkedPool;
    using efficient_pools::PoolHeaderG;

    const size_t __threshold = 128; // malloc performs equally well
                                    // on objects of size > 128
    const size_t __mod = sizeof(void*) - 1;
    const size_t __logOfVoid = std::log2(sizeof(void*));

    std::vector<
        std::unique_ptr<GlobalLinkedPool, FreeDeleter<GlobalLinkedPool>>,
        mallocator<std::unique_ptr<GlobalLinkedPool>>
    > __allocators(__threshold >> __logOfVoid);

    std::unique_ptr<avl_tree, FreeDeleter<avl_tree>>  __mallocedPages(
        avl_init((avl_tree*)std::malloc(sizeof(avl_tree)), NULL)
    );

    inline size_t getAllocatorsIndex(size_t t_size) {
        if (t_size == 0) {
            return 0;
        }
        return (t_size >> __logOfVoid) - 1;
    }
}

inline void* custom_new_no_throw(size_t t_size) {
    // use malloc for large sizes
    if (t_size > __threshold) {
        void* addr = std::malloc(t_size);
        size_t maskedAddr = reinterpret_cast<size_t>(addr) &
            GlobalLinkedPool::POOL_MASK;
        void* page = reinterpret_cast<void*>(maskedAddr);
        auto res = _get_entry(page_get(__mallocedPages.get(), page),
                              PageNode, avl);
        if (res) {
            ++res->num;
        } else {
            page_insert(__mallocedPages.get(), page);
        }
        return addr;
    } else {
        size_t remainder = t_size & __mod; // t_size % sizeof(void*)
        // round up to the next multiple of <sizeof(void*)>
        t_size = remainder == 0 ? t_size : (t_size + __mod) & ~__mod;
        auto& poolAlloc = __allocators[getAllocatorsIndex(t_size)];
        if (poolAlloc) {
            // our pool was already created, just use it
            return poolAlloc->allocate();
        } else {
            // create the pool which can hold objects of size <size>
            poolAlloc.reset(
                new (malloc(sizeof(GlobalLinkedPool))) GlobalLinkedPool(t_size)
            );
            return poolAlloc->allocate();
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
    size_t addr = reinterpret_cast<size_t>(t_ptr);
    void* page = reinterpret_cast<void*>(addr & GlobalLinkedPool::POOL_MASK);
    avl_node* kv = page_get(__mallocedPages.get(), page);
    auto res = _get_entry(kv, PageNode, avl);
    if (res) {
        if (res->num > 1) {
            --res->num;
        } else {
            page_remove(__mallocedPages.get(), kv);
        }
        std::free(t_ptr);
    } else {
        const PoolHeaderG& ph = GlobalLinkedPool::getPoolHeader(t_ptr);
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        __allocators[getAllocatorsIndex(ph.sizeOfObjects)]
            ->deallocate(t_ptr);
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
