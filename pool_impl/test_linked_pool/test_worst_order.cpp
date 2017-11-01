#include <vector>
#include <ctime>
#include <fstream>

#include "TestObject.h"
#include "../src/linked_pool/LinkedPool.h"

using std::vector;

/**
   Allocates a number of TestObjects on the heap and deallocates
   them in an order which causes LinkedPool to work extra.
   Allocation and deallocation is done with both new/delete and LinkedPool.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `worst_time_taken.txt' and
   it will be of the form:
     Allocating <ARG> * 170 objects.
     Allocate TestObject normally: X ms
     Deallocate TestObject normally: Y ms
     Allocate TestObject with LinkedPool: X1 ms
     Deallocate TestObject with LinkedPool: Y1 ms
 */
int main(int argc, char* argv[]) {
    size_t MULT = argc > 1 ? std::stoul(argv[1]) : 10000;
    // XXX hardcoded, do not change
    size_t POOL_SIZE = 170;
    size_t BOUND = POOL_SIZE * MULT;

    std::clock_t start;
    std::ofstream f("worst_time_taken.txt");
    f << "Allocating " << BOUND << " objects." << std::endl;

    {
        vector<TestObject*> objs;
        objs.reserve(BOUND);
        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            objs.push_back(new TestObject());
        }

        f << "Allocate TestObject normally: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;

        start = std::clock();
        for (int i = 0; i < POOL_SIZE; ++i) {
            for (int offset = 0; offset < MULT; ++offset) {
                delete (objs[i + offset * POOL_SIZE]);
            }
        }

        f << "Deallocate TestObject normally: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
    {
        LinkedPool<TestObject> lp;
        vector<TestObject*> objs2;
        objs2.reserve(BOUND);
        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            objs2.push_back((TestObject*) lp.allocate());
        }

        f << "Allocate TestObject with LinkedPool: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;

        start = std::clock();
        for (int i = 0; i < POOL_SIZE; ++i) {
            for (int offset = 0; offset < MULT; ++offset) {
                lp.deallocate(objs2[i + offset * POOL_SIZE]);
            }
        }

        f << "Deallocate TestObject with LinkedPool: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
}
