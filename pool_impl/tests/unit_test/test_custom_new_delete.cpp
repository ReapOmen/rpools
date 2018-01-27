#include "catch.hpp"

#include <vector>
using std::vector;

#include "custom_operators/custom_new_delete_v1/custom_new_delete.h"
using efficient_pools::PoolHeaderG;

TEST_CASE("Allocations between 0 and 128 bytes use GlobalLinkedPool",
          "[custom_new_delete]") {
    void* first = custom_new_no_throw(0);
    for (size_t i = 1; i < sizeof(void*) + 1; ++i) {
        void* ptr = custom_new_no_throw(i);

        REQUIRE((NSGlobalLinkedPool::POOL_MASK & (size_t)first) ==
                (NSGlobalLinkedPool::POOL_MASK & (size_t)ptr));

        // same headers for 0 through 8 because they get allocated
        // in the same pool
        const auto& hLast = NSGlobalLinkedPool::getPoolHeader(first);
        const auto& h = NSGlobalLinkedPool::getPoolHeader(ptr);
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
            REQUIRE((NSGlobalLinkedPool::POOL_MASK & (size_t)first) ==
                    (NSGlobalLinkedPool::POOL_MASK & (size_t)ptr));

            const auto& hLast = NSGlobalLinkedPool::getPoolHeader(first);
            const auto& h = NSGlobalLinkedPool::getPoolHeader(ptr);
            REQUIRE(h == hLast);

            // offset of a certain multiple between each element and the
            // first allocated element
            REQUIRE(((size_t)ptr - (size_t)first) % h.sizeOfSlot == 0);
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
                NSGlobalLinkedPool::getPoolHeader(allocs[i-1]).occupiedSlots;
            custom_delete(allocs[i-1]);
            REQUIRE(oldSize - 1 ==
                    NSGlobalLinkedPool::getPoolHeader(allocs[index]).occupiedSlots);
        }
    }
}
