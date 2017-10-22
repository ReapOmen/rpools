#ifndef __OBJ1_H__
#define __OBJ1_H__

#include <cstdlib>
using std::malloc;
#include <vector>

class Obj1 {
public:

    static size_t overhead, POOL_SIZE, METADATA_SIZE;

    static size_t calculateOverhead();

    static std::pair<void*, size_t> getPool(void* ptr);

    static std::vector<void*> pools;

    static size_t getFreeCount(void* ptr);

    static void* nextFree(void* ptr);

    Obj1() = default;

    ~Obj1() = default;

    static void* operator new(size_t size);

    static void operator delete(void* ptr);

private:
    // 12 bytes
    int _x;
    int _y;
    int _z;
};

#endif // __OBJ1_H__
