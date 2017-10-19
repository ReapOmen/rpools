#include "src/SomeObject.h"
#include "src/SomeObject2.h"
#include "src/SomeObject3.h"
#define BOUND 100000

#include <vector>

int main() {

    // let's create a lot of objects!
    std::vector<SomeObject*> objs;
    std::vector<SomeObject2*> objs2;
    std::vector<SomeObject3*> objs3;
    for (int i = 0; i < BOUND; ++i) {
        // for each SomeObject, there is a 16 bytes overhead -> 16000 bytes of overhead!
        objs.push_back(new SomeObject());
        // for 2 SomeObject2, there is a 16 bytes overhead -> 8000 bytes overhead
        // + sizeof(vector<void*>) + sizeof(map<void*, size_t>) + 8000 * sizeof(pair<void*, size_t>)
        // in this case, SomeObject2 is consuming even more memory, but these objects only get allocated
        // in pools of two objects, my assumption is:
        // if the pool is large enough, then the pool overhead together with the allocation overhead
        // should be a lot smaller then the normal allocation overhead
        objs2.push_back(new SomeObject2());

        objs3.push_back(new SomeObject3());
    }

    // now let's deallocate all of them
    for (int i = 0; i < BOUND; ++i) {
        delete objs.back();
        objs.pop_back();
        delete objs2.back();
        objs2.pop_back();
        delete objs3.back();
        objs3.pop_back();
    }

    return 0;
}
