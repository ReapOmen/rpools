#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <cmath>
#include <vector>
#include <map>

extern "C" {
#include "avltree/avl_utils.h"
}

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

    struct GlobalLinkedPoolDeleter {
        void operator()(GlobalLinkedPool* pool) {
            std::free(pool);
        }
    };

    struct AvlTreeDeleter {
        void operator()(avl_tree* tree) {
            std::free(tree);
        }
    };

    std::vector<std::unique_ptr<GlobalLinkedPool, GlobalLinkedPoolDeleter>,
                mallocator<std::unique_ptr<GlobalLinkedPool>>>
        __allocators(__threshold >> __logOfVoid);

    std::unique_ptr<avl_tree, AvlTreeDeleter>  __mallocedPages(
        avl_init((avl_tree*)std::malloc(sizeof(avl_tree)), NULL)
    );

    // std::map<void*, size_t, std::less<void*>,
    //          mallocator<std::pair<const void*, size_t>>> mallocedPages;

    inline size_t getAllocatorsIndex(size_t size) {
        if (size == 0) {
            return 0;
        }
        return (size >> __logOfVoid) - 1;
    }
}

inline void* custom_new_no_throw(size_t size) {
    // use malloc for large sizes
    if (size > __threshold) {
        void* addr = std::malloc(size);
        void* page = (void*)((size_t)addr & GlobalLinkedPool::POOL_MASK);
        auto res = _get_entry(avl2::get2(__mallocedPages.get(), page), avl2::PageNode, avl);
        if (res) {
            ++res->num;
        } else {
            avl2::insert2(__mallocedPages.get(), page);
        }
        return addr;
    } else {
        size_t remainder = size & __mod; // size % sizeof(void*)
        // round up to the next multiple of <sizeof(void*)>
        size = remainder == 0 ? size : (size + __mod) & ~__mod;
        auto& poolAlloc = __allocators[getAllocatorsIndex(size)];
        if (poolAlloc) {
            // our pool was already created, just use it
            return poolAlloc->allocate();
        } else {
            // create the pool which can hold objects of size <size>
            poolAlloc.reset(
                new (malloc(sizeof(GlobalLinkedPool))) GlobalLinkedPool(size)
            );
            return poolAlloc->allocate();
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
    const PoolHeaderG& ph = GlobalLinkedPool::getPoolHeader(ptr);
    // find out if the pointer was allocated with malloc
    // or within a pool
    void* page = (void*)((size_t)ptr &
                         GlobalLinkedPool::POOL_MASK);
    avl_node* kv = avl2::get2(__mallocedPages.get(), page);
    auto res = _get_entry(kv, avl2::PageNode, avl);
    if (res) {
        if (res->num > 1) {
            --res->num;
        } else {
            avl2::remove2(__mallocedPages.get(), kv);
        }
    } else {
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        __allocators[getAllocatorsIndex(ph.sizeOfObjects)]
            ->deallocate(ptr);
    }
}

#endif // __CUSTOM_NEW_DELETE_H__
