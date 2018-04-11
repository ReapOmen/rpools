#include "rpools/custom_new/custom_new_delete.hpp"
#include "unit_test/TestObject.h"
#include <string>
#include <vector>
using std::vector;

int main(int argc, char* argv[]) {
    size_t BOUND = argc < 2 ? 10000 : std::stoul(argv[1]);
    vector<TestObject*> objs;
    for (size_t i = 0; i < BOUND; ++i) {
        objs.push_back(static_cast<TestObject*>(custom_new(sizeof(TestObject))));
    }
    for (size_t i = 0; i < BOUND; ++i) {
        custom_delete (objs.back());
        objs.pop_back();
    }
}
