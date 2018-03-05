#include "custom_new/custom_new_delete.hpp"

#include <cmath>

#include "tools/FreeDeleter.hpp"
#include "GlobalPools.hpp"
#include "tools/LMLock.hpp"

namespace {
    using namespace rpools;

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

void* custom_new_no_throw(size_t t_size, size_t t_alignment) {
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

void* custom_new(size_t t_size, size_t t_alignment) {
    void* toRet = custom_new_no_throw(t_size, t_alignment);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

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

// list of all new functions:
//   http://en.cppreference.com/w/cpp/memory/new/operator_new
// list of all delete functions:
//   http://en.cppreference.com/w/cpp/memory/new/operator_delete

// Some operators are not implemented because their default
// implementation will not break custom_new/custom_delete

// Note that the C++14/17/20 operators are not included!

void* operator new(std::size_t t_size) {
    return custom_new(t_size);
}

void* operator new(std::size_t t_size, const std::nothrow_t& nothrow_value) noexcept {
    return custom_new_no_throw(t_size);
}

void operator delete(void* t_ptr) noexcept {
    if (t_ptr != nullptr) {
        custom_delete(t_ptr);
    }
}

void operator delete(void* t_ptr, const std::nothrow_t& nothrow_value) noexcept {
    if (t_ptr != nullptr) {
        custom_delete(t_ptr);
    }
}

void* operator new[](std::size_t t_size) {
    void* toRet = custom_new(t_size);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

void* operator new[](std::size_t t_size, const std::nothrow_t& nothrow_value) noexcept {
    return custom_new_no_throw(t_size);
}

void operator delete[](void* t_ptr) noexcept {
    custom_delete(t_ptr);
}

void operator delete[](void* t_ptr, const std::nothrow_t& nothrow_value) noexcept {
    custom_delete(t_ptr);
}
