#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <cmath>

#include "tools/FreeDeleter.hpp"
#include "GlobalPools.hpp"
#include "tools/LMLock.hpp"

namespace {
    using efficient_pools::NSGlobalLinkedPool;

    const size_t __threshold = 128; // malloc performs equally well
                                    // on objects of size > 128
    const size_t __mod = sizeof(void*) - 1;
    const size_t __logOfVoid = std::log2(sizeof(void*));

    GlobalPools __pools(__threshold >> __logOfVoid);

    std::unique_ptr<avl_tree, FreeDeleter<avl_tree>>  __mallocedPages(
        avl_init((avl_tree*)std::malloc(sizeof(avl_tree)), nullptr)
    );

    LMLock __lock;
}

/**
 *  Allocates `t_size` bytes and aligns it according to `t_alignment`.
 *  @note This function will return a nullptr when allocation fails.
 *  @param t_size the size of the allocation
 *  @param t_alignment the alignment of the allocation
 *  @return a pointer aligned to `t_alignment` of size `t_size`.
 */
void* custom_new_no_throw(size_t t_size,
                          size_t t_alignment=alignof(max_align_t)) {
    // use malloc for large sizes or if we are dealing with
    // alignments that are not 2, 4, 8, 16
    if ((alignof(max_align_t) & (t_alignment - 1)) != 0 || t_size > __threshold) {
        void* addr = aligned_alloc(t_alignment, t_size);
        auto maskedAddr = reinterpret_cast<size_t>(addr) &
            NSGlobalLinkedPool::POOL_MASK;
        auto page = reinterpret_cast<void*>(maskedAddr);
        __lock.lock();
        auto res = _get_entry(page_get(__mallocedPages.get(), page),
                              PageNode, avl);
        if (res) {
            ++res->num;
        } else {
            page_insert(__mallocedPages.get(), page);
        }
        __lock.unlock();
        return addr;
    } else {
        size_t remainder = t_size & __mod; // t_size % sizeof(void*)
        // round up to the next multiple of sizeof(void*)
        t_size = remainder == 0 ? t_size : (t_size + __mod) & ~__mod;
        // adds 8 in the case when a pool of <t_size> cannot accommodate
        // an allocation request of alignment <t_alignment>
        // say t_size is 40 and t_alignment is 16
        // we have defined that pools that are not divisible by
        // 16, have alignment 8, otherwise 16
        // 40 % 16 != 0 -> place the request in a pool that holds
        // objects of size 48 (also note 48 % 16 == 0 -> has an alignment of 16)
        t_size += (t_size & (t_alignment - 1)) == 0 ? 0 : sizeof(void*);
        __lock.lock();
        void* addr = __pools.getPool(t_size).allocate();
        __lock.unlock();
        return addr;
    }
}

/**
 *  Allocates `t_size` bytes and aligns it according to `t_alignment`.
 *  @note This function throws bad_alloc when allocation fails.
 *  @param t_size the size of the allocation
 *  @param t_alignment the alignment of the allocation
 *  @return a pointer aligned to `t_alignment` of size `t_size`.
 */
void* custom_new(size_t t_size,
                 size_t t_alignment=alignof(max_align_t)) {
    void* toRet = custom_new_no_throw(t_size, t_alignment);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

/**
 *  Frees up the memory that starts at `t_ptr`.
 *  @param t_ptr the pointer that is freed
 */
void custom_delete(void* t_ptr) noexcept {
    // find out if the pointer was allocated with malloc
    // or within a pool
    auto addr = reinterpret_cast<size_t>(t_ptr);
    auto page = reinterpret_cast<void*>(addr & NSGlobalLinkedPool::POOL_MASK);
    __lock.lock();
    avl_node* kv = page_get(__mallocedPages.get(), page);
    auto res = _get_entry(kv, PageNode, avl);
    if (res) {
        if (res->num > 1) {
            --res->num;
        } else {
            page_remove(__mallocedPages.get(), kv);
        }
        __lock.unlock();
        std::free(t_ptr);
        return;
    } else {
        const PoolHeaderG& ph = NSGlobalLinkedPool::getPoolHeader(t_ptr);
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        __pools.getPool(ph.sizeOfSlot).deallocate(t_ptr);
    }
    __lock.unlock();
}

#endif // __CUSTOM_NEW_DELETE_H__
