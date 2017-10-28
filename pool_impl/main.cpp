#include <vector>
#include <ctime>
#include <fstream>
#include <string>
#include <iostream>

#include "src/bit_pool/BitPool.h"
#include "src/linked_pool/LinkedPool.h"

using std::cout;
using std::endl;

class Test {
    int x, y, z, x1;
};

int main(int argc, char *argv[]) {

    size_t BOUND = 10000;
    size_t POOL_SIZE = 80;

    if (argc < 2) {
        return 1;
    } else {
        BOUND = std::stoul(argv[1]);
        if (argc > 2) {
            POOL_SIZE = std::stoul(argv[2]);
        }
    }

    std::clock_t start;
    std::ofstream f("time_taken.txt");
    f << "Allocating " << BOUND << " objects." << std::endl;

    start = std::clock();
    // let's create a lot of objects!
    std::vector<Test*> objs(BOUND);
    for (int i = 0; i < BOUND; ++i) {
        objs.push_back(new Test());
    }
    f << "Allocate Test normally: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    BitPool<Test> bp(POOL_SIZE);
    start = std::clock();
    // let's create a lot of objects!
    std::vector<Test*> objs2(BOUND);
    for (int i = 0; i < BOUND; ++i) {
        objs2.push_back(bp.allocate());
    }
    f << "Allocate Test with BitPool: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    LinkedPool<Test> lp;
    start = std::clock();
    // let's create a lot of objects!
    std::vector<Test*> objs3(BOUND);
    for (int i = 0; i < BOUND; ++i) {
        objs3.push_back(lp.allocate());
    }
    f << "Allocate Test with LinkedPool: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    // now let's deallocate all of them
    start = std::clock();
    for (int i = 0; i < BOUND; ++i) {
        delete objs.back();
        objs.pop_back();
    }
    f << "Deallocate Test normally: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    start = std::clock();
    for (int i = 0; i < BOUND; ++i) {
        bp.deallocate(objs2.back());
        objs2.pop_back();
    }
    f << "Deallocate Test with BitPool: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    start = std::clock();
    for (int i = 0; i < BOUND; ++i) {
        lp.deallocate(objs3.back());
        objs3.pop_back();
    }
    f << "Deallocate Test with LinkedPool: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    return 0;
}
