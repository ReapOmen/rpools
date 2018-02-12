#include "custom_new_delete_debug.h"
#include "tools/AllocCollector.h"

namespace {
    AllocCollector ac;
}

void* custom_new_no_throw(size_t size, size_t alignment, const char* name) {
    void* toRet = malloc(size);
    ac.addObject(size, alignment, name, toRet);
    return toRet;
}

void* custom_new(size_t size, size_t alignment, const char* name) {
    void* toRet = custom_new_no_throw(size, alignment, name);
    if (toRet == nullptr) {
        throw std::bad_alloc();
    }
    return toRet;
}

void custom_delete(void* ptr) throw() {
    ac.removeObject(ptr);
    free(ptr);
}
