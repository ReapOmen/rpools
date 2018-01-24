#ifndef __TEST_OBJECT_2_H__
#define __TEST_OBJECT_2_H__

#include <cstddef>

class TestObject2 {
public:
    size_t x;
    int z, a, b, c;
private:
    float w, w1, w2;
    virtual void print() {
        (void)x;
        (void)z;
        (void)a;
        (void)b;
        (void)c;
        (void)w;
        (void)w1;
        (void)w2;
    }
};

#endif // __TEST_OBJECT_2_H__
