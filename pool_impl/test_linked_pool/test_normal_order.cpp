#include <vector>
#include <ctime>
#include <fstream>
#include <string>
#include <iostream>

#include "TestObject.h"
#include "../src/linked_pool/LinkedPool.h"

/**
   Allocates a number of TestObjects on the heap and deallocates
   them in reverse order of insertion.
   Allocation and deallocation is done with both new/delete and LinkedPool.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `time_taken.txt' and
   it will be of the form:
     Allocating <ARG> objects.
     Allocate TestObject normally: X ms
     Deallocate TestObject normally: Y ms
     Allocate TestObject with LinkedPool: X1 ms
     Deallocate TestObject with LinkedPool: Y1 ms
 */
int main(int argc, char *argv[]) {
    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 100000;

    std::clock_t start;
    std::ofstream f("time_taken.txt");
    f << "Allocating " << BOUND << " objects." << std::endl;

    start = std::clock();

    {
        std::vector<TestObject*> objs;
        objs.reserve(BOUND);
        for (int i = 0; i < BOUND; ++i) {
            objs.push_back(new TestObject());
        }
        f << "Allocate TestObject normally: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;

        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            delete objs.back();
            objs.pop_back();
        }
        f << "Deallocate Test normally: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
    {
        LinkedPool<TestObject> lp;
        start = std::clock();
        std::vector<TestObject*> objs;
        objs.reserve(BOUND);
        for (int i = 0; i < BOUND; ++i) {
            objs.push_back((TestObject*) lp.allocate());
        }
        f << "Allocate Test with LinkedPool: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;

        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            lp.deallocate(objs.back());
            objs.pop_back();
        }
        f << "Deallocate Test with LinkedPool: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
    return 0;
}
