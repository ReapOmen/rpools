#ifndef __SOME_OBJECT2_H__
#define __SOME_OBJECT2_H__

#include <cstdlib>
#include <vector>
#include <map>

class SomeObject2 {
public:

    static size_t overhead;

    static std::map<void*, size_t> pools;

    static std::vector<void*> free;

    SomeObject2() = default;

    ~SomeObject2() = default;

    static void* operator new(size_t size);

    static void operator delete(void* ptr);

private:
    // 12 bytes
    int _x;
    int _y;
    int _z;
};

#endif // __SOME_OBJECT_H__
