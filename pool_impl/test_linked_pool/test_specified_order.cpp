#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#include "Utility.h"
#include "unit_test/TestObject.h"
#include "linked_pool/LinkedPool.h"

using efficient_pools::LinkedPool;
using std::vector;

void allocateN(size_t num, vector<TestObject*>& vec) {
    for (int i = 0; i < num; ++i) {
        vec.push_back(new TestObject());
    }
}

void deallocateN(size_t num, vector<TestObject*>& vec) {
    for (int i = 0; i < num; ++i) {
        delete vec.back();
        vec.pop_back();
    }
}

void allocateN(size_t num, vector<TestObject*>& vec, LinkedPool<TestObject>& lp) {
    for (int i = 0; i < num; ++i) {
        vec.push_back((TestObject*) lp.allocate());
    }
}

void deallocateN(size_t num, vector<TestObject*>& vec, LinkedPool<TestObject>& lp) {
    for (int i = 0; i < num; ++i) {
        lp.deallocate(vec.back());
        vec.pop_back();
    }
}

/**
   Allocates and deallocates a number of TestObjects on the heap in a certain order.
   Allocation and deallocation is done with both new/delete and LinkedPool.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `specified_time_taken.txt' and
   it will be of the form:
     Allocating <ARG> objects.
     Regular: X ms
     LinkedPool: Y ms
 */
int main(int argc, char *argv[]) {
    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 10000;

    std::clock_t start;
    std::ofstream f("specified_time_taken.txt");

    size_t five = BOUND * 5 / 100;
    size_t ten = BOUND / 10;
    size_t fifteen = BOUND * 15 / 100;
    f << "Allocating " << BOUND << " objects." << std::endl;
    {
        vector<TestObject*> objs;
        objs.reserve(BOUND);
        start = std::clock();

        allocateN(ten, objs);
        deallocateN(five, objs);
        allocateN(five, objs);
        deallocateN(five, objs);

        allocateN(ten, objs);
        deallocateN(five, objs);
        allocateN(five, objs);
        deallocateN(five, objs);

        deallocateN(ten, objs);
        allocateN(five, objs);
        allocateN(five, objs);
        deallocateN(five, objs);

        allocateN(five, objs);
        deallocateN(five, objs);
        allocateN(five, objs);
        deallocateN(ten, objs);

        f << "Regular: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
    {
        LinkedPool<TestObject> lp;
        vector<TestObject*> objs;
        start = std::clock();

        allocateN(ten, objs, lp);
        deallocateN(five, objs, lp);
        allocateN(five, objs, lp);
        deallocateN(five, objs, lp);

        allocateN(ten, objs, lp);
        deallocateN(five, objs, lp);
        allocateN(five, objs, lp);
        deallocateN(five, objs, lp);

        deallocateN(ten, objs, lp);
        allocateN(five, objs, lp);
        allocateN(five, objs, lp);
        deallocateN(five, objs, lp);

        allocateN(five, objs, lp);
        deallocateN(five, objs, lp);
        allocateN(five, objs, lp);
        deallocateN(ten, objs, lp);

        f << "LinkedPool: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
    return 0;
}
