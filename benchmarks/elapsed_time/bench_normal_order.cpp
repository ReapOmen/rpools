/**
 *  @file bench_normal_order.cpp
 *  Allocates a number of `TestObject`s on the heap and deallocates
 *  them in the same order.
 *  Allocation and deallocation is done with `new/delete`, `LinkedPools`
 *  `MemoryPool` and `boost::object_pool`.
 *  @par
 *  A command line argument can be passed to set the number of `TestObject`s
 *  that will be created and destroyed.
 *  @par
 *  The results will be written to a file called **normal_time_taken.json**.
 *  @see JSONWriter
 */

#include <vector>
#include <string>
#include <iostream>

#include "Utility.h"
#include "unit_test/TestObject.h"
#ifdef INCLUDE_BOOST
#include <boost/pool/object_pool.hpp>
#endif
#include "rpools/allocators/MemoryPool.h"
#include "rpools/allocators/LinkedPool.hpp"

using rpools::LinkedPool;

/**
 *  Allocate and deallocate a number of `TestObject`s by using a pool allocator.
 *  @note
 *  The allocation and deallocation times are recorded and written to a file.
 *  @tparam T the type of the pool allocator
 *  @param bound the number of (de)allocations
 *  @param j the JSONWriter which records the speed
 *  @param name the name of the pool allocator which (de)allocates
 *                `TestObject`s
 */
template<template <typename> class T>
void benchPool(size_t bound, JSONWriter& j, const std::string& name) {
    T<TestObject> lp;
    std::vector<TestObject*> objs(bound);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        objs[i] = reinterpret_cast<TestObject*>(lp.allocate());
    }
    j.addAllocation(name, start);

    start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        lp.deallocate(objs[i]);
    }
    j.addDeallocation(name, start);
}

#ifdef INCLUDE_BOOST
template<template <typename, typename> class T>
void benchPool(size_t bound, JSONWriter& j, const std::string& name) {
    T<TestObject, boost::default_user_allocator_malloc_free> bp;
    std::vector<TestObject*> objs(bound);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        objs[i] = reinterpret_cast<TestObject*>(bp.malloc());
    }
    j.addAllocation(name, start);

    start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        bp.free(objs[i]);
    }
    j.addDeallocation(name, start);
}
#endif

int main(int argc, char *argv[]) {
    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 10000;
    JSONWriter j("normal_time_taken.json", BOUND);
    {
        std::vector<TestObject*> objs(BOUND);
        std::clock_t start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs[i] = new TestObject();
        }
        j.addAllocation("new/delete", start);

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            delete objs[i];
        }
        j.addDeallocation("new/delete", start);
    }
    {
        benchPool<LinkedPool>(BOUND, j, "LinkedPool");
    }
    {
        benchPool<MemoryPool>(BOUND, j, "MemoryPool");
    }
#ifdef INCLUDE_BOOST
    {
        benchPool<boost::object_pool>(BOUND, j, "boost::object_pool");
    }
#endif
    return 0;
}
