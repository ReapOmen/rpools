/**
 *  @file bench_random_order.cpp
 *  Allocates a number of `TestObject`s on the heap and deallocates
 *  them in a random order. Allocation and deallocation is done
 *  with `new/delete`, `LinkedPools`, `MemoryPool` and
 *  `boost::object_pool`.
 *  @par
 *  A command line argument can be passed to set the number of `TestObject`s
 *  that will be created and destroyed.
 *  The results will be written to a file called **random_time_taken.json**.
 *  @see JSONWriter
 */

#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#include "Utility.h"
#include "unit_test/TestObject.h"
#ifdef INCLUDE_BOOST
#include <boost/pool/object_pool.hpp>
#endif
#include "pool_allocators/MemoryPool.h"
#include "pool_allocators/LinkedPool.hpp"
#include "pool_allocators/LinkedPool3.hpp"

using rpools::LinkedPool;
using rpools::LinkedPool3;
using std::vector;

/**
 *  Allocate and deallocate a number of `TestObject`s by using a pool allocator.
 *  The deallocation sequence is determined by the `randomPos` vector.
 *  @note
 *  The allocation and deallocation times are recorded and written to a file.
 *  @tparam T the type of the pool allocator
 *  @param bound the number of (de)allocations
 *  @param j the JSONWriter which records the speed
 *  @param randomPos the order in which the objects are deallocated
 *  @param name the name of the pool allocator which (de)allocates
 *              `TestObject`s
 */
template<template <typename> class T>
void benchPool(size_t bound, JSONWriter& j,
               const vector<size_t>& randomPos,
               const std::string& name) {
    T<TestObject> lp;
    vector<TestObject*> objs(bound);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        objs[i] = reinterpret_cast<TestObject*>(lp.allocate());
    }
    j.addAllocation(name, start);

    start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        lp.deallocate(objs[randomPos[i]]);
    }
    j.addDeallocation(name, start);
}

#ifdef INCLUDE_BOOST
template<template <typename, typename> class T>
void benchPool(size_t bound, JSONWriter& j,
               const vector<size_t>& randomPos,
               const std::string& name) {
    T<TestObject, boost::default_user_allocator_malloc_free> lp;
    vector<TestObject*> objs(bound);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        objs[i] = reinterpret_cast<TestObject*>(lp.malloc());
    }
    j.addAllocation(name, start);

    start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        lp.free(objs[randomPos[i]]);
    }
    j.addDeallocation(name, start);
}
#endif

int main(int argc, char *argv[]) {
    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 10000;
    size_t SEED = std::chrono::system_clock::now().time_since_epoch().count();
    JSONWriter j("random_time_taken.json", BOUND);
    // random deallocation indices
    vector<size_t> randomPos(BOUND);
    for (size_t i = 0; i < BOUND; ++i) {
        randomPos[i] = i;
    }
    std::shuffle(randomPos.begin(), randomPos.end(), std::default_random_engine(SEED));
    {
        vector<TestObject*> objs(BOUND);
        std::clock_t start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs[i] = new TestObject();
        }
        j.addAllocation("new/delete", start);

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            delete objs[randomPos[i]];
        }
        j.addDeallocation("new/delete", start);
    }
    {
        benchPool<LinkedPool>(BOUND, j, randomPos, "LinkedPool");
    }
    {
        benchPool<LinkedPool3>(BOUND, j, randomPos, "LinkedPool3");
    }
    {
        benchPool<MemoryPool>(BOUND, j, randomPos, "MemoryPool");
    }
#ifdef INCLUDE_BOOST
    {
        benchPool<boost::object_pool>(BOUND, j, randomPos, "boost::object_pool");
    }
#endif
    return 0;
}
