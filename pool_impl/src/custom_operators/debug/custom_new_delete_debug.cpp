#include "custom_new_delete_debug.h"
#include "tools/AllocCollector.h"

namespace {
    AllocCollector ac;
}

void* custom_new_no_throw(size_t t_size, size_t t_alignment,
                          const char* t_name, size_t t_baseSize) {
    void* toRet = malloc(t_size);
    ac.addObject(t_size, t_alignment, t_name, t_baseSize, toRet);
    return toRet;
}

void* custom_new(size_t t_size, size_t t_alignment,
                 const char* t_name, size_t t_baseSize) {
    void* toRet = custom_new_no_throw(t_size, t_alignment, t_name, t_baseSize);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

void custom_delete(void* t_ptr) throw() {
    ac.removeObject(t_ptr);
    free(t_ptr);
}
