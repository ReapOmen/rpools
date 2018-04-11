#include <memory>
#include <cassert>
#include "rpools/allocators/GlobalLinkedPool.hpp"
using rpools::GlobalLinkedPool;

// note that NSGlobalLinkedPool = GlobalLinkedPool without thread safety 
int main() {
    // this is typeless, it can be used to allocate space for any
    // object whose sizeof is smaller than 4
    GlobalLinkedPool lp(4); // alignment is alignof(max_align_t)
    auto x = static_cast<int*>(lp.allocate()); // can hold ints
    auto y = static_cast<float*>(lp.allocate()); // floats as well
    lp.deallocate(x);
    lp.deallocate(y);

    GlobalLinkedPool lp2(8, 4);
    // OK
    auto isFine = static_cast<std::pair<int, int>*>(lp2.allocate());
    // BAD IDEA!
    auto misalignedDouble = static_cast<double*>(lp2.allocate());
    lp2.deallocate(isFine);
    lp2.deallocate(misalignedDouble);
}
