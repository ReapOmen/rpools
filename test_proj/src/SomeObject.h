#ifndef __SOME_OBJECT_H__
#define __SOME_OBJECT_H__

#include <cstdlib>

class SomeObject {
public:

    SomeObject() = default;

    ~SomeObject() = default;

    static void* operator new(size_t size);

    static void operator delete(void* ptr);

private:
    // 12 bytes
    int _x;
    int _y;
    int _z;
};

#endif // __SOME_OBJECT_H__
