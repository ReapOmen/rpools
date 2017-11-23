#include "custom_new_delete.h"

void * operator new(std::size_t n) throw(std::bad_alloc) {
    return custom_new(n);
}

void operator delete(void * p) throw() {
    custom_delete(p);
}
