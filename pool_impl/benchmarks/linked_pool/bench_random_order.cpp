#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#include "Utility.h"
#include "unit_test/TestObject.h"
#include "pool_allocators/MemoryPool.h"
#include "pool_allocators/LinkedPool.h"
#include "pool_allocators/LinkedPool3.h"

using efficient_pools::LinkedPool;
using efficient_pools3::LinkedPool3;
using std::vector;

template<template <typename> class T>
void benchPool(size_t BOUND, std::ofstream& f,
               const vector<size_t>& randomPos, const std::string& name) {
    T<TestObject> lp;
    vector<TestObject*> objs;
    objs.reserve(BOUND);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < BOUND; ++i) {
        objs.push_back((TestObject*) lp.allocate());
    }
    printToFile(f, "TestObject", start, false, name);

    start = std::clock();
    for (size_t i = 0; i < BOUND; ++i) {
        lp.deallocate(objs[randomPos[i]]);
    }
    printToFile(f, "TestObject", start, true, name);
}

/**
   Allocates a number of TestObjects on the heap and deallocates
   them in a random order. Allocation and deallocation is done
   with both new/delete and LinkedPools.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `random_time_taken.output' and
   it will be of the form:
     Allocating <ARG> objects.
     Allocate TestObject normally: X ms
     Deallocate TestObject normally: Y ms
     Allocate TestObject with LinkedPool: X1 ms
     Deallocate TestObject with LinkedPool: Y1 ms
     ...
 */
int main(int argc, char *argv[]) {
    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 10000;
    size_t SEED = std::chrono::system_clock::now().time_since_epoch().count();

    std::ofstream f("random_time_taken.output");
    f << "Allocating " << BOUND << " objects." << std::endl;

    // random deallocation indices
    vector<size_t> randomPos;
    randomPos.reserve(BOUND);
    for (size_t i = 0; i < BOUND; ++i) {
        randomPos.push_back(i);
    }
    std::shuffle(randomPos.begin(), randomPos.end(), std::default_random_engine(SEED));

    {
        vector<TestObject*> objs;
        objs.reserve(BOUND);
        std::clock_t start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs.push_back(new TestObject());
        }
        printToFile(f, "TestObject", start, false, "Regular");

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            delete objs[randomPos[i]];
        }
        printToFile(f, "TestObject", start, true, "Regular");
    }
    {
        benchPool<LinkedPool>(BOUND, f, randomPos, "LinkedPool");
    }
    {
        benchPool<LinkedPool3>(BOUND, f, randomPos, "LinkedPool3");
    }
    {
        benchPool<MemoryPool>(BOUND, f, randomPos, "MemoryPool");
    }
    return 0;
}
