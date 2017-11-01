#include <vector>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>

#include "TestObject.h"
#include "../src/linked_pool/LinkedPool.h"

using std::vector;

/**
   Allocates a number of TestObjects on the heap and deallocates
   them in a random order. Allocation and deallocation is done
   with both new/delete and LinkedPool.
   A command line argument can be passed to set the number of TestObjects
   that will be created and destroyed.
   The results will be written to a file called `random_time_taken.txt' and
   it will be of the form:
     Allocating <ARG> objects.
     Allocate TestObject normally: X ms
     Deallocate TestObject normally: Y ms
     Allocate TestObject with LinkedPool: X1 ms
     Deallocate TestObject with LinkedPool: Y1 ms
 */
int main(int argc, char *argv[]) {

    size_t BOUND = argc > 1 ? std::stoul(argv[1]) : 10000;
    size_t SEED = std::chrono::system_clock::now().time_since_epoch().count();

    std::clock_t start;
    std::ofstream f("random_time_taken.txt");
    f << "Allocating " << BOUND << " objects." << std::endl;

    // random deallocation indices
    vector<size_t> randomPos;
    randomPos.reserve(BOUND);
    for (size_t i = 0; i < BOUND; ++i) {
        randomPos.push_back(i);
    }
    std::shuffle(randomPos.begin(), randomPos.end(), std::default_random_engine(SEED));

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
        for (int i = 0; i < BOUND; ++i) {
            delete objs[randomPos[i]];
        }
        f << "Deallocate TestObject normally: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
    {
        LinkedPool<TestObject> lp;
        vector<TestObject*> objs;
        objs.reserve(BOUND);
        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            objs.push_back((TestObject*) lp.allocate());
        }
        f << "Allocate TestObject with LinkedPool: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;

        start = std::clock();
        for (int i = 0; i < BOUND; ++i) {
            lp.deallocate(objs[randomPos[i]]);
        }
        f << "Deallocate TestObject with LinkedPool: "
          << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
          << " ms" << std::endl;
    }
    return 0;
}
