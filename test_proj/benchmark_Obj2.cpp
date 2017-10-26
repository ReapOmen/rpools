#include "src/Obj2.h"
#include "src/SomeObject.h"

#include <ctime>
#include <vector>
using std::vector;

#include <iostream>
using std::cout;
using std::endl;

int main() {
    const size_t BOUND = 100000;
    {
        vector<Obj2*> objs(BOUND);
        std::clock_t start;

        cout << "Timing the allocation of " << BOUND << " Obj2s" << endl;

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs.push_back(new Obj2());
        }
        cout << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
             << " ms" << endl;

        cout << "Timing the deallocation of " << BOUND << " Obj2s" << endl;

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            delete objs.back();
            objs.pop_back();
        }
        cout << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
             << " ms" << endl;
    }
    {
        vector<SomeObject*> objs(BOUND);
        std::clock_t start;

        cout << "Timing the allocation of " << BOUND << " SomeObjects" << endl;

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            objs.push_back(new SomeObject());
        }
        cout << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
             << " ms" << endl;

        cout << "Timing the deallocation of " << BOUND << " SomeObjects" << endl;

        start = std::clock();
        for (size_t i = 0; i < BOUND; ++i) {
            delete objs.back();
            objs.pop_back();
        }
        cout << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
             << " ms" << endl;
    }
}
