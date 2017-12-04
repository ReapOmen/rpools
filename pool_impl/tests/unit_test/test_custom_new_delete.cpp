#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
using std::vector;

#include "custom_new_delete.h"
using efficient_pools::PoolHeaderG;

TEST_CASE("Allocations between 1 and 128 bytes use GlobalLinkedPool",
          "[custom_new_delete]") {
    for (size_t i = 0; i <= 120; i += 8) {
        void* lastPtr = nullptr;
        // allocations are done in pools of multiples of 8
        // 0-8 -> 8
        // 9-16 -> 16
        // etc.
        // note that 0 will also be placed in a pool of size 8
        // because its a special case
        for (size_t j = i == 0 ? i : i + 1; j <= i + 8; ++j) {
            void* ptr = custom_new_no_throw(j);
            if (!lastPtr) {
                lastPtr = ptr;
            } else {
                REQUIRE((GlobalLinkedPool::POOL_MASK & (size_t)lastPtr) ==
                        (GlobalLinkedPool::POOL_MASK & (size_t)ptr));

                const auto& hLast = GlobalLinkedPool::getPoolHeader(lastPtr);
                const auto& h = GlobalLinkedPool::getPoolHeader(ptr);
                REQUIRE(h == hLast);
            }
        }
    }
}

TEST_CASE("Allocations over 128 bytes use malloc",
          "[custom_new_delete]") {
    // the uperbound can be anything
    for (size_t i = 129; i < 1000; ++i) {
        void* ptr = custom_new_no_throw(i);
        REQUIRE(strcmp(GlobalLinkedPool::getPoolHeader(ptr).isPool,
                       PoolHeaderG::IS_POOL) != 0);
    }
}

TEST_CASE("Pointers to objects of size < 129 are deallocated using GlobalLinkedpool",
          "[custom_new_delete]") {
    std::vector<void*> allocs(129);
    for (size_t i = 0; i <= 128; ++i) {
        allocs[i] = custom_new_no_throw(i);
    }
    for (size_t i = allocs.size(); i != 0; --i) {
        // we do not want to deal with dangling references
        // because our pool header will get destroyed once we
        // deallocate the last object of the pool
        if ((i - 1) % 8 == 0 && (i - 1) != 0) {
            custom_delete(allocs[i-1]);
        } else {
            size_t oldSize =
                GlobalLinkedPool::getPoolHeader(allocs[i-1]).sizeOfPool;
            custom_delete(allocs[i-1]);
            REQUIRE(oldSize - 1 ==
                    GlobalLinkedPool::getPoolHeader(allocs[i-1]).sizeOfPool);
        }
    }
}
