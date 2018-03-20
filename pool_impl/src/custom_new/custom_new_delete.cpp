#include "rpools/custom_new/custom_new_delete.hpp"

#include <cmath>
#include <cstring>
#include <iostream>

#include "GlobalPools.hpp"
#include "rpools/tools/LMLock.hpp"

namespace {
    using namespace rpools;

    const size_t __threshold = 128; // malloc performs equally well
                                    // on objects of size > 128
    const size_t __mod = sizeof(void*) - 1;
    const size_t __logOfVoid = std::log2(sizeof(void*));

    struct MallocHeader {
        char validity[16] = "              \0";
    };

    GlobalPools& getPools() {
        static GlobalPools pools(__threshold >> __logOfVoid);
        return pools;
    }

    LMLock& getLock() {
        static LMLock lock;
        return lock;
    }
}

void* custom_new_no_throw(size_t t_size, size_t t_alignment) {
    LMLock& lock = getLock();
    // use malloc for large sizes or if we are dealing with
    // alignments that are not 2, 4, 8, 16
    if (mod(alignof(max_align_t), t_alignment) != 0 || t_size > __threshold) {
        auto addr = static_cast<char*>(std::malloc(t_size +
                                                   sizeof(MallocHeader)));
        auto header = new(addr) MallocHeader();
        std::strcpy(header->validity, "IsThIsMaLlOcD!\0");
        return addr + sizeof(MallocHeader);
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
        t_size += (mod(t_size, t_alignment)) == 0 ? 0 : sizeof(void*);
        lock.lock();
        void* addr = getPools().getPool(t_size).allocate();
        lock.unlock();
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
    auto cAddr = reinterpret_cast<char*>(t_ptr);
    cAddr -= sizeof(MallocHeader);
    auto header = reinterpret_cast<MallocHeader*>(cAddr);
    if (std::strcmp(header->validity, "IsThIsMaLlOcD!\0") == 0) {
        free(cAddr);
    } else {
        LMLock& lock = getLock();
        lock.lock();
        const PoolHeaderG& ph = NSGlobalLinkedPool::getPoolHeader(t_ptr);
        // convert the size to an index of the allocators vector
        // by dividing it to sizeof(void*)
        getPools().getPool(ph.sizeOfSlot).deallocate(t_ptr);
        lock.unlock();
    }
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
