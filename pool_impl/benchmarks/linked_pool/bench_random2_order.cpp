#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstdlib>
#include <time.h>

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
using std::pair;
using std::make_pair;
using std::vector;

float allocateN(const pair<int, int>& range, vector<TestObject*>& vec) {
    std::clock_t start = std::clock();
    for (int i = range.first; i < range.second; ++i) {
        vec[i] = new TestObject();
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

float deallocateN(size_t index, vector<TestObject*>& vec) {
    std::clock_t start = std::clock();
    delete vec[index];
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

template<template <typename> class T>
float allocateN(const pair<int, int>& range, vector<TestObject*>& vec,
               T<TestObject>& lp) {
    std::clock_t start = std::clock();
    for (int i = range.first; i < range.second; ++i) {
        vec[i] = (TestObject*) lp.allocate();
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

template<template <typename> class T>
float deallocateN(size_t index, vector<TestObject*>& vec, T<TestObject>& lp) {
    std::clock_t start = std::clock();
    lp.deallocate(vec[index]);
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

size_t SEED = std::chrono::system_clock::now().time_since_epoch().count();

void shuffle(vector<size_t>& vec) {
    std::shuffle(vec.begin(), vec.end(),
                 std::default_random_engine(SEED));
}

void pushAndPop(vector<pair<size_t, bool>>& vec,
                size_t deallocNum,
                vector<size_t>& allocated) {
    for (int i = 0; i < deallocNum; ++i) {
        vec.push_back(make_pair(allocated.back(), true));
        allocated.pop_back();
    }
}

template<template <typename> class T>
void benchPool(size_t BOUND, std::ofstream& f,
               const vector<pair<size_t, bool>>& order,
               const std::string& name) {
    T<TestObject> lp;
    vector<TestObject*> objs(BOUND);
    float alloc = 0.0f;
    float dealloc = 0.0f;
    size_t startIndex = 0;
    for (const auto& pair : order) {
        if (!pair.second) {
            alloc += allocateN(make_pair(startIndex, startIndex + pair.first), objs, lp);
            startIndex += pair.first;
        } else {
            dealloc += deallocateN(pair.first, objs, lp);
        }
    }
    printToFile(f, "TestObject", alloc, false, name);
    printToFile(f, "TestObject", dealloc, true, name);
}

/**
   Allocates and deallocates a number of TestObjects on the heap in a certain
   order. This order is defined as follows: take a random number N < BOUND
   (which is the argument provided to the executable) and allocate N objects.
   After that another random number M < N is generated which specifies how
   many random deallocations must be made from the curret set of allocated
   objects.
   This is done until BOUND objects have been allocate and deallocated.
   The order of (de)allocation is saved so that both implementations will
   (de)allocate in the same order.
   Allocation and deallocation is done with both new/delete and LinkedPools.
   The results will be written to a file called `random2_time_taken.txt' and
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
    std::ofstream f("random2_time_taken.txt");

    time_t seconds;
    time(&seconds);
    srand((unsigned int) seconds);

    // keeps the order of allocation/deallocation
    vector<pair<size_t, bool>> order;
    // used to make sure we do not deallocate the same pointer twice
    vector<size_t> allocated;
    allocated.reserve(BOUND);
    // we loop until we have allocated BOUND objects
    size_t rangeStart = 0;
    while (rangeStart < BOUND) {
        size_t allocation = rand() % (BOUND - rangeStart) + 1;
        size_t rangeEnd = rangeStart + allocation;
        for (size_t i = rangeStart; i < rangeEnd; ++i) {
            // push index where the new object is stored
            allocated.push_back(i);
        }
        // shuffle the indices in order to have a random
        // deallocation mechanism
        shuffle(allocated);
        rangeStart = rangeEnd;
        // record how many objects we allocated
        order.push_back(make_pair(allocation, false));
        // record how many random deallocations have been made and
        // at what indices
        pushAndPop(order, rand() % allocated.size() + 1, allocated);
    }
    pushAndPop(order, allocated.size(), allocated);

    f << "Allocating " << BOUND << " objects." << std::endl;
    {
        vector<TestObject*> objs(BOUND);
        float alloc = 0.0f;
        float dealloc = 0.0f;
        size_t startIndex = 0;
        for (const auto& pair : order) {
            if (!pair.second) {
                alloc += allocateN(make_pair(startIndex, startIndex + pair.first), objs);
                startIndex += pair.first;
            } else {
                dealloc += deallocateN(pair.first, objs);
            }
        }
        printToFile(f, "TestObject", alloc, false, "Regular");
        printToFile(f, "TestObject", dealloc, true, "Regular");
    }
    {
        benchPool<LinkedPool>(BOUND, f, order, "LinkedPool");
    }
    {
        benchPool<LinkedPool2>(BOUND, f, order, "LinkedPool2");
    }
    {
        benchPool<LinkedPool3>(BOUND, f, order, "LinkedPool3");
    }
    {
        benchPool<LinkedPool4>(BOUND, f, order, "LinkedPool4");
    }
    return 0;
}
