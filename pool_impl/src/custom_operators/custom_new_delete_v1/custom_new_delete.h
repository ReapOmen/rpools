#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <cmath>
#include <vector>

#include "tools/mallocator.h"
#include "tools/FreeDeleter.h"
#include "pool_allocators/NSGlobalLinkedPool.h"

#ifdef __x86_64
#include "tools/light_lock.h"
#else
#include <mutex>
#include <thread>
#endif

namespace {
    using efficient_pools::NSGlobalLinkedPool;
    using efficient_pools::PoolHeaderG;

    const size_t __threshold = 128; // malloc performs equally well
                                    // on objects of size > 128
    const size_t __mod = sizeof(void*) - 1;
    const size_t __logOfVoid = std::log2(sizeof(void*));

    std::vector<
        std::unique_ptr<NSGlobalLinkedPool, FreeDeleter<NSGlobalLinkedPool>>,
        mallocator<std::unique_ptr<NSGlobalLinkedPool>>
    > __allocators(__threshold >> __logOfVoid);

    std::unique_ptr<avl_tree, FreeDeleter<avl_tree>>  __mallocedPages(
        avl_init((avl_tree*)std::malloc(sizeof(avl_tree)), NULL)
    );

#ifdef __x86_64
    light_lock_t __lock(LIGHT_LOCK_INIT);
#else
    std::mutex __lock;
#endif

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
            NSGlobalLinkedPool::POOL_MASK;
        void* page = reinterpret_cast<void*>(maskedAddr);
#ifdef __x86_64
        light_lock(&__lock);
#else
        std::lock_guard<std::mutex> lock(__lock);
#endif
        auto res = _get_entry(page_get(__mallocedPages.get(), page),
                              PageNode, avl);
        if (res) {
            ++res->num;
        } else {
            page_insert(__mallocedPages.get(), page);
        }
#ifdef __x86_64
        light_unlock(&__lock);
#endif
        return addr;
    } else {
        size_t remainder = t_size & __mod; // t_size % sizeof(void*)
        // round up to the next multiple of <sizeof(void*)>
        t_size = remainder == 0 ? t_size : (t_size + __mod) & ~__mod;
#ifdef __x86_64
        light_lock(&__lock);
#else
        std::lock_guard<std::mutex> lock(__lock);
#endif
        auto& poolAlloc = __allocators[getAllocatorsIndex(t_size)];
        if (!poolAlloc) {
            // create the pool which can hold objects of size <size>
            poolAlloc.reset(
                new (malloc(sizeof(NSGlobalLinkedPool)))
                    NSGlobalLinkedPool(t_size)
            );
        }
        void* toRet = poolAlloc->allocate();
#ifdef __x86_64
        light_unlock(&__lock);
#endif
        return toRet;
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
    void* page = reinterpret_cast<void*>(addr & NSGlobalLinkedPool::POOL_MASK);
#ifdef __x86_64
    light_lock(&__lock);
#else
    std::unique_lock<std::mutex> lock(__lock);
#endif
    avl_node* kv = page_get(__mallocedPages.get(), page);
    auto res = _get_entry(kv, PageNode, avl);
    if (res) {
        if (res->num > 1) {
            --res->num;
        } else {
            page_remove(__mallocedPages.get(), kv);
        }
#ifdef __x86_64
        light_unlock(&__lock);
#else
        lock.unlock();
#endif
        std::free(t_ptr);
        return;
    } else {
        const PoolHeaderG& ph = NSGlobalLinkedPool::getPoolHeader(t_ptr);
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        __allocators[getAllocatorsIndex(ph.sizeOfObjects)]
            ->deallocate(t_ptr);
    }
#ifdef __x86_64
    light_unlock(&__lock);
#else
    lock.unlock();
#endif
}

#endif // __CUSTOM_NEW_DELETE_H__
