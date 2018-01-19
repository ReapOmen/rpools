#include <vector>
#include <string>
#include <iostream>

#include "Utility.h"
#include "unit_test/TestObject.h"
#include "pool_allocators/LinkedPool.h"
#include "pool_allocators/LinkedPool3.h"

using efficient_pools::LinkedPool;
using efficient_pools3::LinkedPool3;

template<template <typename> class T>
void benchPool(size_t BOUND, std::ofstream& f, const std::string& name) {
    T<TestObject> lp;
    std::vector<TestObject*> objs;
    objs.reserve(BOUND);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < BOUND; ++i) {
        objs.push_back((TestObject*) lp.allocate());
    }
    printToFile(f, "TestObject", start, false, name);

    start = std::clock();
    for (size_t i = 0; i < BOUND; ++i) {
        lp.deallocate(objs.back());
        objs.pop_back();
    }
    printToFile(f, "TestObject", start, true, name);
}

/**
   Allocates a number of TestObjects on the heap and deallocates
   them in reverse order of insertion.
   Allocation and deallocation is done with both new/delete and LinkedPools.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `normal_time_taken.output' and
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

    std::ofstream f("normal_time_taken.output");
    f << "Allocating " << BOUND << " objects." << std::endl;
    {
        std::vector<TestObject*> objs;
        objs.reserve(BOUND);
        std::clock_t start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs.push_back(new TestObject());
        }
        printToFile(f, "TestObject", start, false, "Regular");

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            delete objs.back();
            objs.pop_back();
        }
        printToFile(f, "TestObject", start, true, "Regular");
    }
    {
        benchPool<LinkedPool>(BOUND, f, "LinkedPool");
    }
    {
        benchPool<LinkedPool3>(BOUND, f, "LinkedPool3");
    }
    return 0;
}
