#ifndef __TEST_OBJECT_H__
#define __TEST_OBJECT_H__

struct TestObject {
    int x, y, z;
    virtual void print() {}
    virtual ~TestObject() = default;
};

#endif // __TEST_OBJECT_H__
