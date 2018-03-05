#include <string>

#include "unit_test/TestObject.h"
#include "pool_allocators/LinkedPool.hpp"
using rpools::LinkedPool;

#include <vector>
using std::vector;

int main(int argc, char* argv[]) {
    size_t BOUND = argc < 2 ? 10000 : std::stoul(argv[1]);
    LinkedPool<TestObject> lp;
    vector<TestObject*> objs;
    for (size_t i = 0; i < BOUND; ++i) {
        objs.push_back(static_cast<TestObject*>(lp.allocate()));
    }
    for (size_t i = 0; i < BOUND; ++i) {
        lp.deallocate(objs.back());
        objs.pop_back();
    }
}
