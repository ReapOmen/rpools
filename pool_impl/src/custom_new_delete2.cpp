#include "custom_new_delete2.h"

// list of all new functions:
//   http://en.cppreference.com/w/cpp/memory/new/operator_new
// list of all delete functions:
//   http://en.cppreference.com/w/cpp/memory/new/operator_delete

// Some operators are not implemented because their default
// implementation will not break custom_new/custom_delete

// Note that the C++14/17/20 operators are not included!

void* operator new(std::size_t size) {
    return custom_new(size);
}

void* operator new(std::size_t size, const std::nothrow_t& nothrow_value) noexcept {
    return custom_new_no_throw(size);
}

void operator delete(void* ptr) noexcept {
    if (ptr != nullptr) {
        custom_delete(ptr);
    }
}

void operator delete(void* ptr, const std::nothrow_t& nothrow_value) noexcept {
    if (ptr != nullptr) {
        custom_delete(ptr);
    }
}

void* operator new[](std::size_t size) {
    void* toRet = std::malloc(size);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

void* operator new[](std::size_t size, const std::nothrow_t& nothrow_value) noexcept {
    return std::malloc(size);
}

void operator delete[](void* ptr) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t& nothrow_value) noexcept {
    std::free(ptr);
}
