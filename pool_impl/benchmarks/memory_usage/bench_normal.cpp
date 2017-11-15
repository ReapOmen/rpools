#include "Tools.h"
#include "unit_test/TestObject.h"
#include <vector>
using std::vector;

int main(int argc, char* argv[]) {
    size_t BOUND = argc < 2 ? 10000 : std::stoul(argv[1]);
    vector<TestObject*> objs;
    for (size_t i = 0; i < BOUND; ++i) {
        objs.push_back(new TestObject());
    }
    for (size_t i = 0; i < BOUND; ++i) {
        delete (objs.back());
        objs.pop_back();
    }
}
