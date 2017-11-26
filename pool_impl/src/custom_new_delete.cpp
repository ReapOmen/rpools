#include "custom_new_delete.h"

void* operator new(std::size_t n) {
    return custom_new(n);
}

void* operator new(std::size_t size, const std::nothrow_t& nothrow_value) noexcept {
    return custom_new_no_throw(size);
}

void operator delete(void * p) noexcept {
    custom_delete(p);
}

void* operator new[](std::size_t n) {
    return std::malloc(n);
}

void* operator new[](std::size_t size, const std::nothrow_t& nothrow_value) noexcept {
    return std::malloc(size);
}

void operator delete[](void* ptr) noexcept {
    std::free(ptr);
}
