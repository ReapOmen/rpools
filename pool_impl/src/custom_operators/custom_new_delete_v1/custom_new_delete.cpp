#include "custom_new_delete.h"

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
