#include <vector>
#include <string>
#include <iostream>

#include "Utility.h"
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
        printToFile(f, "TestObject", start, false, false);

        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            delete objs.back();
            objs.pop_back();
        }
        printToFile(f, "TestObject", start, true, false);
    }
    {
        LinkedPool<TestObject> lp;
        start = std::clock();
        std::vector<TestObject*> objs;
        objs.reserve(BOUND);
        for (int i = 0; i < BOUND; ++i) {
            objs.push_back((TestObject*) lp.allocate());
        }
        printToFile(f, "TestObject", start, false, true);

        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            lp.deallocate(objs.back());
            objs.pop_back();
        }
        printToFile(f, "TestObject", start, true, true);
    }
    return 0;
}
