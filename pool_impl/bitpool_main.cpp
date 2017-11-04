#include "src/bit_pool/BitPool.h"
#include <vector>
using std::vector;

#include <iostream>
using std::cout;
using std::endl;

struct Test {
    Test() = default;
    int x, y, z;
    virtual void v() {

    }
};

int main(int agrc, char* argv[]) {
    size_t BOUND = 10000;
    BitPool<Test> pool = BitPool<Test>(80);
    vector<Test*> objs;
    objs.reserve(BOUND);
    for (int i = 0; i < BOUND; ++i) {
        objs.push_back(pool.allocate());
        objs.back()->v();
    }
    for (int i = 0; i < BOUND; ++i) {
        pool.deallocate(objs.back());
        objs.pop_back();
    }
    return 0;
}
