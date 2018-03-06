/**
 *  @file bench_random2_order.cpp
 *  Allocates and deallocates a number of `TestObject`s on the heap in a certain
 *  order. This order is defined as follows: take a random number **N < BOUND**
 *  (which is the argument provided to the executable) and allocate `N` objects.
 *  After that another random number **M < N** is generated which specifies how
 *  many random deallocations must be made from the curret set of allocated
 *  objects.
 *  This is done until `BOUND` objects have been allocated and deallocated.
 *  @par
 *  The order of (de)allocation is saved so that all implementations will
 *  (de)allocate in the same order.
 *  @par
 *  Allocation and deallocation is done with `new/delete`, `LinkedPools`,
 *  `MemoryPool` and `boost::object_pool`.
 *  @par
 *  The results will be written to a file called **random2_time_taken.json**
 *  @see JSONWriter
 */

#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstdlib>
#include <ctime>

#include "Utility.h"
#include "unit_test/TestObject.h"
#ifdef INCLUDE_BOOST
#include <boost/pool/object_pool.hpp>
#endif
#include "rpools/allocators/MemoryPool.h"
#include "rpools/allocators/LinkedPool.hpp"
#include "rpools/allocators/LinkedPool3.hpp"

using rpools::LinkedPool;
using rpools::LinkedPool3;
using std::pair;
using std::make_pair;
using std::vector;

/**
 *  Allocates `num` `TestObject`s using `new` which are stored in `vec`.
 *  @param range the range of indices where to store in `vec`
 *  @param vec the vector in which the allocations are saved
 *  @return The number of ms it took to allocate `num` `TestObject`s.
 */
float allocateN(const pair<size_t, size_t>& range, vector<TestObject*>& vec) {
    std::clock_t start = std::clock();
    for (size_t i = range.first; i < range.second; ++i) {
        vec[i] = new TestObject();
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

/**
 *  Deallocates the `TestObject` at a given index in `vec` by using `delete`.
 *  @param index the index in `vec`
 *  @param vec the vector from which the `TestObject` is `delete`d
 *  @return The number of ms it took to deallocate the `TestObject`.
 */
float deallocateN(size_t index, vector<TestObject*>& vec) {
    std::clock_t start = std::clock();
    delete vec[index];
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

/**
 *  Allocates a number `TestObject`s using a pool allocator which
 *  are stored in `vec`.
 *  @tparam T the type of the pool allocator
 *  @param range the range of indices where to store in `vec`
 *  @param vec the vector in which the allocations are saved
 *  @param lp the pool allocator
 *  @return The number of ms it took to allocate the `TestObject`s.
 */
template<template <typename> class T>
float allocateN(const pair<size_t, size_t>& range, vector<TestObject*>& vec,
               T<TestObject>& lp) {
    std::clock_t start = std::clock();
    for (size_t i = range.first; i < range.second; ++i) {
        vec[i] = (TestObject*) lp.allocate();
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

/**
 *  Deallocates the `TestObject` at a given index in `vec` by using a pool
 *  allocator.
 *  @param index the index in `vec`
 *  @param vec the vector from which the `TestObject` is deallocated
 *  @return The number of ms it took to deallocate the `TestObject`.
 */
template<template <typename> class T>
float deallocateN(size_t index, vector<TestObject*>& vec, T<TestObject>& lp) {
    std::clock_t start = std::clock();
    lp.deallocate(vec[index]);
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

#ifdef INCLUDE_BOOST
template<template <typename, typename> class T>
float allocateN(const pair<size_t, size_t>& range, vector<TestObject*>& vec,
                T<TestObject, boost::default_user_allocator_malloc_free>& lp) {
    std::clock_t start = std::clock();
    for (size_t i = range.first; i < range.second; ++i) {
        vec[i] = (TestObject*) lp.malloc();
    }
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

template<template <typename, typename> class T>
float deallocateN(size_t index, vector<TestObject*>& vec,
                  T<TestObject, boost::default_user_allocator_malloc_free>& lp) {
    std::clock_t start = std::clock();
    lp.free(vec[index]);
    return (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
}

template<template <typename, typename> class T>
void benchPool(size_t BOUND, JSONWriter& j,
               const vector<pair<size_t, bool>>& order,
               const std::string& name) {
    T<TestObject, boost::default_user_allocator_malloc_free> lp;
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
    j.addAllocation(name, alloc);
    j.addDeallocation(name, dealloc);
}
#endif

/** Used for shuffling. */
size_t SEED = std::chrono::system_clock::now().time_since_epoch().count();

/**
 *  Shuffle the given vector according to `SEED`.
 *  @param vec the vector that is shuffled
 */
void shuffle(vector<size_t>& vec) {
    std::shuffle(vec.begin(), vec.end(),
                 std::default_random_engine(SEED));
}

/**
 *  Pushes a number of deallocation indices into `vec`.
 *  @param vec the vector in which the deallocation index is pushed into
 *  @param deallocNum the number of values that are poped back from
 *                      `allocated`
 *  @param allocated the indices which contain allocated `TestObject`s
 */
void pushAndPop(vector<pair<size_t, bool>>& vec,
                size_t deallocNum,
                vector<size_t>& allocated) {
    for (size_t i = 0; i < deallocNum; ++i) {
        vec.emplace_back(allocated.back(), true);
        allocated.pop_back();
    }
}

/**
 *  Allocate and deallocate a number of `TestObject`s by using a pool allocator.
 *  The deallocation sequence is determined by the `order` vector.
 *  @note
 *  The allocation and deallocation times are recorded and written to a file.
 *  @tparam T the type of the pool allocator
 *  @param bound the number of (de)allocations
 *  @param j the JSONWriter which records the speed
 *  @param order the order in which the objects are (de)allocated
 *  @param name the name of the pool allocator which (de)allocates
 *              `TestObject`s
 */
template<template <typename> class T>
void benchPool(size_t bound, JSONWriter& j,
               const vector<pair<size_t, bool>>& order,
               const std::string& name) {
    T<TestObject> lp;
    vector<TestObject*> objs(bound);
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
    j.addAllocation(name, alloc);
    j.addDeallocation(name, dealloc);
}

int main(int argc, char *argv[]) {
    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 10000;

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
        order.emplace_back(allocation, false);
        // record how many random deallocations have been made and
        // at what indices
        pushAndPop(order, rand() % allocated.size() + 1, allocated);
    }
    pushAndPop(order, allocated.size(), allocated);

    JSONWriter j("random2_time_taken.json", BOUND);
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
        j.addAllocation("new/delete", alloc);
        j.addDeallocation("new/delete", dealloc);
    }
    {
        benchPool<LinkedPool>(BOUND, j, order, "LinkedPool");
    }
    {
        benchPool<LinkedPool3>(BOUND, j, order, "LinkedPool3");
    }
    {
        benchPool<MemoryPool>(BOUND, j, order, "MemoryPool");
    }
#ifdef INCLUDE_BOOST
    {
        benchPool<boost::object_pool>(BOUND, j, order, "boost::object_pool");
    }
#endif
    return 0;
}
