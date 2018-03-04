#include "catch.hpp"

#include <vector>
using std::vector;

#include "custom_operators/custom_new_delete_v1/custom_new_delete.hpp"
#include "pool_allocators/NSGlobalLinkedPool.hpp"
using efficient_pools::NSGlobalLinkedPool;

TEST_CASE("Allocations between 0 and 128 bytes have correct alignment",
          "[custom_new_delete]") {
    for (size_t i = 0; i <= 128; ++i) {
        void* ptr = custom_new_no_throw(i, sizeof(void*));
        REQUIRE((size_t)ptr % sizeof(void*) == 0);
    }
}

TEST_CASE("Pointers to objects of size < 129 are deallocated using GlobalLinkedPool",
          "[custom_new_delete]") {
    std::vector<void*> allocs(129);
    for (size_t i = 0; i <= 128; ++i) {
        allocs[i] = custom_new_no_throw(i, sizeof(void*));
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

TEST_CASE("Weird alignments are correctly accommodated",
          "[custom_new_delete]") {
    REQUIRE((size_t)custom_new(8) % 16 == 0);
    REQUIRE((size_t)custom_new(120, 32) % 32 == 0);
    REQUIRE((size_t)custom_new(500, 64) % 64 == 0);
    REQUIRE((size_t)custom_new(48, 16) % 16 == 0);
}

TEST_CASE("121 aligned at 16 bytes will get allocated into a pool of size 128",
          "[custom_new_delete]") {
    void* res = custom_new_no_throw(121, 16);
    REQUIRE((size_t)res % 16 == 0);
    REQUIRE(NSGlobalLinkedPool::getPoolHeader(res).sizeOfSlot == 128);
}
