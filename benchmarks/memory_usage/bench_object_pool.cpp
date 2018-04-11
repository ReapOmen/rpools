#include <string>

#include "unit_test/TestObject.h"
#include <boost/pool/object_pool.hpp>
using boost::object_pool;

#include <vector>
using std::vector;

int main(int argc, char* argv[]) {
    size_t BOUND = argc < 2 ? 10000 : std::stoul(argv[1]);
    object_pool<TestObject, boost::default_user_allocator_malloc_free> lp;
    vector<TestObject*> objs;
    for (size_t i = 0; i < BOUND; ++i) {
        objs.push_back(static_cast<TestObject*>(lp.malloc()));
    }
    for (size_t i = 0; i < BOUND; ++i) {
        lp.free(objs.back());
        objs.pop_back();
    }
}
