#include <vector>
#include <ctime>
#include <fstream>

#include "src/SomeObject.h"
#include "src/SomeObject2.h"
#include "src/SomeObject3.h"

#define BOUND 100000

int main() {
    std::clock_t start;
    std::ofstream f("time_taken.txt");
    f << "Allocating " << BOUND << " objects." << std::endl;

    start = std::clock();
    // let's create a lot of objects!
    std::vector<SomeObject*> objs;
    for (int i = 0; i < BOUND; ++i) {
        objs.push_back(new SomeObject());
    }
    f << "Allocate SomeObject1: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    start = std::clock();
    std::vector<SomeObject2*> objs2;
    for (int i = 0; i < BOUND; ++i) {
        objs2.push_back(new SomeObject2());
    }
    f << "Allocate SomeObject2: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    start = std::clock();
    std::vector<SomeObject3*> objs3;
    for (int i = 0; i < BOUND; ++i) {
        objs3.push_back(new SomeObject3());
    }
    f << "Allocate SomeObject3: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    // now let's deallocate all of them
    start = std::clock();
    for (int i = 0; i < BOUND; ++i) {
        delete objs.back();
        objs.pop_back();
    }
    f << "Deallocate SomeObject1: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    start = std::clock();
    for (int i = 0; i < BOUND; ++i) {
        delete objs2.back();
        objs2.pop_back();
    }
    f << "Deallocate SomeObject2: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    start = std::clock();
    for (int i = 0; i < BOUND; ++i) {
        delete objs3.back();
        objs3.pop_back();
    }
    f << "Deallocate SomeObject3: "
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;

    return 0;
}
