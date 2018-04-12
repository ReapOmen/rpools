#ifndef __TEST_OBJECT_H__
#define __TEST_OBJECT_H__

#include <cstddef>

struct TestObject {
    size_t x;
    int z;
    virtual void print() {
        (void)x;
        (void)z;
    }
    virtual ~TestObject() = default;
};

#endif // __TEST_OBJECT_H__
