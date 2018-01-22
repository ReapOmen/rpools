#include <vector>

#include "Utility.h"
#include "unit_test/TestObject.h"
#include "pool_allocators/MemoryPool.h"
#include "pool_allocators/LinkedPool.h"
#include "pool_allocators/LinkedPool3.h"

using efficient_pools::LinkedPool;
using efficient_pools3::LinkedPool3;
using std::vector;

template<template <typename> class T>
void benchPool(size_t BOUND, std::ofstream& f, size_t POOL_SIZE,
               size_t MULT, const std::string& name) {
    T<TestObject> lp;
    vector<TestObject*> objs2;
    objs2.reserve(BOUND);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < BOUND; ++i) {
        objs2.push_back((TestObject*) lp.allocate());
    }
    printToFile(f, "TestObject", start, false,  name);

    start = std::clock();
    for (size_t i = 0; i < POOL_SIZE; ++i) {
        for (size_t offset = 0; offset < MULT; ++offset) {
            lp.deallocate(objs2[i + offset * POOL_SIZE]);
        }
    }
    printToFile(f, "TestObject", start, true, name);
}

/**
   Allocates a number of TestObjects on the heap and deallocates
   them in an order which causes LinkedPool to work extra.
   Allocation and deallocation is done with both new/delete and LinkedPools.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `worst_time_taken.output' and
   it will be of the form:
     Allocating <ARG> * 170 objects.
     Allocate TestObject normally: X ms
     Deallocate TestObject normally: Y ms
     Allocate TestObject with LinkedPool: X1 ms
     Deallocate TestObject with LinkedPool: Y1 ms
     ...
 */
int main(int argc, char* argv[]) {
    size_t MULT = argc > 1 ? std::stoul(argv[1]) : 10000;
    // every type of linked pool will provide the same pool size
    size_t POOL_SIZE = LinkedPool<TestObject>().getPoolSize();
    size_t BOUND = POOL_SIZE * MULT;

    std::ofstream f("worst_time_taken.output");
    f << "Allocating " << BOUND << " objects." << std::endl;
    {
        vector<TestObject*> objs;
        objs.reserve(BOUND);
        std::clock_t start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs.push_back(new TestObject());
        }
        printToFile(f, "TestObject", start, false, "Regular");

        start = std::clock();
        for (size_t i = 0; i < POOL_SIZE; ++i) {
            for (size_t offset = 0; offset < MULT; ++offset) {
                delete (objs[i + offset * POOL_SIZE]);
            }
        }
        printToFile(f, "TestObject", start, true, "Regular");
    }
    {
        benchPool<LinkedPool>(BOUND, f, POOL_SIZE, MULT, "LinkedPool");
    }
    {
        benchPool<LinkedPool3>(BOUND, f, POOL_SIZE, MULT, "LinkedPool3");
    }
    {
        benchPool<MemoryPool>(BOUND, f, POOL_SIZE, MULT, "MemoryPool");
    }
}
