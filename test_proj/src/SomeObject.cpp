#include "SomeObject.h"
#include "AllocFile.h"

#include <iostream>
using std::cout;
using std::endl;


using ::allocFile;
using std::max;

size_t SomeObject::count = 0;
size_t SomeObject::overhead = 0;

void* SomeObject::operator new(size_t size) {
    allocFile.processAllocation(size);
    ++count;
    overhead = max(overhead, 16*count);
    return std::malloc(size);
}

void SomeObject::operator delete(void *ptr) {
    allocFile.processDeallocation(sizeof(SomeObject));
    std::free(ptr);
    --count;
    overhead = max(overhead, 16*count);
}
