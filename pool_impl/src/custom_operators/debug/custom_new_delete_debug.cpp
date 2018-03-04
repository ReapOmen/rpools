#include "custom_operators/debug/custom_new_delete_debug.hpp"
#include "AllocCollector.hpp"

namespace {
    AllocCollector ac;
}

void* custom_new_no_throw(size_t t_size, size_t t_alignment,
                          const char* t_name, size_t t_baseSize,
                          const char* t_funcName) {
    void* toRet = malloc(t_size);
    ac.addObject(t_size, t_alignment, t_name, t_baseSize, t_funcName, toRet);
    return toRet;
}

void* custom_new(size_t t_size, size_t t_alignment,
                 const char* t_name, size_t t_baseSize,
                 const char* t_funcName) {
    void* toRet = custom_new_no_throw(t_size, t_alignment, t_name,
                                      t_baseSize, t_funcName);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

void custom_delete(void* t_ptr) noexcept {
    ac.removeObject(t_ptr);
    free(t_ptr);
}
