#ifndef __OBJ3_H__
#define __OBJ3_H__

#include <cstdlib>
#include <vector>
#define WRITE_ALLOCS_TO_FILE 1

#ifdef WRITE_ALLOCS_TO_FILE
#include "AllocFile.h"
#endif

struct Node {
    Node* next;
};

class Obj3 {
public:

    static size_t POOL_SIZE, METADATA_SIZE;
#ifdef WRITE_ALLOCS_TO_FILE
    static size_t overhead;

    static size_t calculateOverhead();
#endif
    static void* getPool(void* ptr);

    static std::vector<void*> pools;

    static size_t getFreeCount(void* ptr);

    static void* nextFree(void* ptr);

    Obj3() = default;

    ~Obj3() = default;

    static void* operator new(size_t size);

    static void operator delete(void* ptr);

private:
    // 12 bytes
    int _x;
    int _y;
    int _z;
};

#endif // __OBJ3_H__
