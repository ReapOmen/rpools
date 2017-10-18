#include "SomeObject.h"
#include "AllocFile.h"

using ::allocFile;

void* SomeObject::operator new(size_t size) {
    allocFile.processAllocation(size);
    return std::malloc(size);
}

void SomeObject::operator delete(void *ptr) {
    allocFile.processDeallocation(sizeof(SomeObject));
    std::free(ptr);
}
