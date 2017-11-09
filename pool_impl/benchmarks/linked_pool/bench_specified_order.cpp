#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#include "Utility.h"
#include "unit_test/TestObject.h"
#include "linked_pool/LinkedPool.h"
#include "linked_pool/LinkedPool2.h"
#include "linked_pool/LinkedPool3.h"
#include "linked_pool/LinkedPool4.h"

using efficient_pools::LinkedPool;
using efficient_pools2::LinkedPool2;
using efficient_pools3::LinkedPool3;
using efficient_pools4::LinkedPool4;
using std::vector;

float allocateN(size_t num, vector<TestObject*>& vec) {
    std::clock_t start = std::clock();
    for (int i = 0; i < num; ++i) {
        vec.push_back(new TestObject());
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

float deallocateN(size_t num, vector<TestObject*>& vec) {
    std::clock_t start = std::clock();
    for (int i = 0; i < num; ++i) {
        delete vec.back();
        vec.pop_back();
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

template<template <typename> class T>
float allocateN(size_t num, vector<TestObject*>& vec, T<TestObject>& lp) {
    std::clock_t start = std::clock();
    for (int i = 0; i < num; ++i) {
        vec.push_back((TestObject*) lp.allocate());
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

template<template <typename> class T>
float deallocateN(size_t num, vector<TestObject*>& vec, T<TestObject>& lp) {
    std::clock_t start = std::clock();
    for (int i = 0; i < num; ++i) {
        lp.deallocate(vec.back());
        vec.pop_back();
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

template<template <typename> class T>
void benchPool(size_t BOUND, std::ofstream& f,
               size_t five, size_t ten,
               const std::string& name) {
    T<TestObject> lp;
    vector<TestObject*> objs;

    float alloc = 0.0f;
    float dealloc = 0.0f;

    alloc += allocateN(ten, objs, lp);
    dealloc += deallocateN(five, objs, lp);
    alloc += allocateN(five, objs, lp);
    dealloc += deallocateN(five, objs, lp);

    alloc += allocateN(ten, objs, lp);
    dealloc += deallocateN(five, objs, lp);
    alloc += allocateN(five, objs, lp);
    dealloc += deallocateN(five, objs, lp);

    dealloc += deallocateN(ten, objs, lp);
    alloc += allocateN(five, objs, lp);
    alloc += allocateN(five, objs, lp);
    dealloc += deallocateN(five, objs, lp);

    alloc += allocateN(five, objs, lp);
    dealloc += deallocateN(five, objs, lp);
    alloc += allocateN(five, objs, lp);
    dealloc += deallocateN(ten, objs, lp);

    printToFile2(f, "TestObject", alloc, false, name);
    printToFile2(f, "TestObject", dealloc, true, name);
}

/**
   Allocates and deallocates a number of TestObjects on the heap in a certain order.
   Allocation and deallocation is done with both new/delete and LinkedPools.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `specified_time_taken.txt' and
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

    std::clock_t start;
    std::ofstream f("specified_time_taken.txt");

    size_t five = BOUND * 5 / 100;
    size_t ten = BOUND / 10;
    f << "Allocating " << BOUND << " objects." << std::endl;
    {
        vector<TestObject*> objs;
        objs.reserve(BOUND);
        float alloc = 0.0f;
        float dealloc = 0.0f;

        alloc += allocateN(ten, objs);
        dealloc += deallocateN(five, objs);
        alloc += allocateN(five, objs);
        dealloc += deallocateN(five, objs);

        alloc += allocateN(ten, objs);
        dealloc += deallocateN(five, objs);
        alloc += allocateN(five, objs);
        dealloc += deallocateN(five, objs);

        dealloc += deallocateN(ten, objs);
        alloc += allocateN(five, objs);
        alloc += allocateN(five, objs);
        dealloc += deallocateN(five, objs);

        alloc += allocateN(five, objs);
        dealloc += deallocateN(five, objs);
        alloc += allocateN(five, objs);
        dealloc += deallocateN(ten, objs);

        printToFile2(f, "TestObject", alloc, false, "Regular");
        printToFile2(f, "TestObject", dealloc, true, "Regular");
    }
    {
        benchPool<LinkedPool>(BOUND, f, five, ten, "LinkedPool");
    }
    {
        benchPool<LinkedPool2>(BOUND, f, five, ten, "LinkedPool2");
    }
    {
        benchPool<LinkedPool3>(BOUND, f, five, ten, "LinkedPool3");
    }
    {
        benchPool<LinkedPool4>(BOUND, f, five, ten, "LinkedPool4");
    }
    return 0;
}
