/**
 *  @file bench_normal_order.cpp
 *  Allocates a number of `TestObject`s on the heap and deallocates
 *  them in the same order.
 *  Allocation and deallocation is done with both `new/delete`, `LinkedPools`
 *  `MemoryPool` and `boost::object_pool`.
 *  @par
 *  A command line argument can be passed to set the number of `TestObject`s
 *  that will be created and destroyed.
 *  @par
 *  The results will be written to a file called **normal_time_taken.output** and
 *  it will be of the form: <br>
 *    ```
 *    Allocating <ARG> objects.
 *    Allocate TestObject normally: X ms
 *    Deallocate TestObject normally: Y ms
 *    Allocate TestObject with LinkedPool: X1 ms
 *    Deallocate TestObject with LinkedPool: Y1 ms
 *    ...
 *    ```
 */

#include <vector>
#include <string>
#include <iostream>

#include "Utility.h"
#include "unit_test/TestObject.h"
#ifdef INCLUDE_BOOST
#include <boost/pool/object_pool.hpp>
#endif
#include "pool_allocators/MemoryPool.h"
#include "pool_allocators/LinkedPool.h"
#include "pool_allocators/LinkedPool3.h"

using efficient_pools::LinkedPool;
using efficient_pools3::LinkedPool3;

/**
 *  Allocate and deallocate a number of `TestObject`s by using a pool allocator.
 *  @note
 *  The allocation and deallocation times are recorded and written to a file.
 *  @tparam T the type of the pool allocator
 *  @param bound the number of (de)allocations
 *  @param f the file in which the output is written
 *  @param name the name of the pool allocator which (de)allocates
 *                `TestObject`s
 */
template<template <typename> class T>
void benchPool(size_t bound, std::ofstream& f, const std::string& name) {
    T<TestObject> lp;
    std::vector<TestObject*> objs(bound);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        objs[i] = reinterpret_cast<TestObject*>(lp.allocate());
    }
    printToFile(f, "TestObject", start, false, name);

    start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        lp.deallocate(objs[i]);
    }
    printToFile(f, "TestObject", start, true, name);
}

#ifdef INCLUDE_BOOST
template<template <typename, typename> class T>
void benchPool(size_t bound, std::ofstream& f, const std::string& name) {
    T<TestObject, boost::default_user_allocator_malloc_free> bp;
    std::vector<TestObject*> objs(bound);
    std::clock_t start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        objs[i] = reinterpret_cast<TestObject*>(bp.malloc());
    }
    printToFile(f, "TestObject", start, false, name);

    start = std::clock();
    for (size_t i = 0; i < bound; ++i) {
        bp.free(objs[i]);
    }
    printToFile(f, "TestObject", start, true, name);
}
#endif

int main(int argc, char *argv[]) {
    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 10000;

    std::ofstream f("normal_time_taken.output");
    f << "Allocating " << BOUND << " objects." << std::endl;
    {
        std::vector<TestObject*> objs(BOUND);
        std::clock_t start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs[i] = new TestObject();
        }
        printToFile(f, "TestObject", start, false, "Regular");

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            delete objs[i];
        }
        printToFile(f, "TestObject", start, true, "Regular");
    }
    {
        benchPool<LinkedPool>(BOUND, f, "LinkedPool");
    }
    {
        benchPool<LinkedPool3>(BOUND, f, "LinkedPool3");
    }
    {
        benchPool<MemoryPool>(BOUND, f, "MemoryPool");
    }
#ifdef INCLUDE_BOOST
    {
        benchPool<boost::object_pool>(BOUND, f, "boost::object_pool");
    }
#endif
    return 0;
}
