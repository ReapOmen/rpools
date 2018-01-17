#include "catch.hpp"

#include <vector>
using std::vector;

#include "custom_new_delete.h"
using efficient_pools::PoolHeaderG;

TEST_CASE("Allocations between 0 and 128 bytes use GlobalLinkedPool",
          "[custom_new_delete]") {
    void* first = custom_new_no_throw(0);
    for (size_t i = 1; i < sizeof(void*) + 1; ++i) {
        void* ptr = custom_new_no_throw(i);

        REQUIRE((GlobalLinkedPool::POOL_MASK & (size_t)first) ==
                (GlobalLinkedPool::POOL_MASK & (size_t)ptr));

        // same headers for 0 through 8 because they get allocated
        // in the same pool
        const auto& hLast = GlobalLinkedPool::getPoolHeader(first);
        const auto& h = GlobalLinkedPool::getPoolHeader(ptr);
        REQUIRE(h == hLast);

        // an offset of 8 is required between each slot
        REQUIRE((void*)((char*)first + i * sizeof(void*)) == ptr);
    }
    for (size_t multiple = 1; multiple <= 1; ++multiple) {
        size_t start = sizeof(void*) * multiple;
        size_t end = sizeof(void*) * (multiple + 1);
        first = custom_new_no_throw(start + 1);
        for (size_t size = start + 2; size <= end; ++size) {
            // allocations are done in pools of multiples of 8
            // 9-16 -> 16
            // etc.
            void* ptr = custom_new_no_throw(size);
            REQUIRE((GlobalLinkedPool::POOL_MASK & (size_t)first) ==
                    (GlobalLinkedPool::POOL_MASK & (size_t)ptr));

            const auto& hLast = GlobalLinkedPool::getPoolHeader(first);
            const auto& h = GlobalLinkedPool::getPoolHeader(ptr);
            REQUIRE(h == hLast);

            size_t elem = size - start - 1;
            REQUIRE((void*)((char*)first + elem * end) == ptr);
        }
    }
}

TEST_CASE("Pointers to objects of size < 129 are deallocated using GlobalLinkedPool",
          "[custom_new_delete]") {
    std::vector<void*> allocs(129);
    for (size_t i = 0; i <= 128; ++i) {
        allocs[i] = custom_new_no_throw(i);
    }
    size_t index = allocs.size() - 1;
    for (size_t i = allocs.size(); i > 0; --i) {
        // we do not want to deal with dangling references
        // because our pool header will get destroyed once we
        // deallocate the last object of the pool
        if ((i - 1) % 8 == 1) {
            custom_delete(allocs[i-1]);
            index = i - 2;
        } else {
            size_t oldSize =
                GlobalLinkedPool::getPoolHeader(allocs[i-1]).sizeOfPool;
            custom_delete(allocs[i-1]);
            REQUIRE(oldSize - 1 ==
                    GlobalLinkedPool::getPoolHeader(allocs[index]).sizeOfPool);
        }
    }
}
