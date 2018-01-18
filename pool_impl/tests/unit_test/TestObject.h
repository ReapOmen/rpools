#ifndef __TEST_OBJECT_H__
#define __TEST_OBJECT_H__

struct TestObject {
    int x, y, z;
    virtual void print() {
        (void)x;
        (void)y;
        (void)z;
    }
    virtual ~TestObject() = default;
};

#endif // __TEST_OBJECT_H__
