#ifndef __SOME_OBJECT3_H__
#define __SOME_OBJECT3_H__

#include <cstdlib>
using std::malloc;
#include <fstream>
#include <vector>
#include <map>

class SomeObject3 {
public:

    static size_t overhead, POOL_SIZE;

    static std::map<void*, size_t> pools;

    static std::vector<void*> free;

    SomeObject3() = default;

    ~SomeObject3() = default;

    static void* operator new(size_t size);

    static void operator delete(void* ptr);

private:
    // 12 bytes
    int _x;
    int _y;
    int _z;
};

#endif // __SOME_OBJECT3_H__
