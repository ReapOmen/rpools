#ifndef __SOME_OBJECT2_H__
#define __SOME_OBJECT2_H__

#include <cstdlib>
using std::malloc;
#include <fstream>
#include <vector>
#include <map>

class SomeObject2 {
public:

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
