#include <memory>
#include <cassert>
#include "rpools/allocators/LinkedPool.hpp"
using rpools::LinkedPool;
#include "SomeObject.hpp"

int main() {
    LinkedPool<SomeObject> lp;
    auto so = static_cast<SomeObject*>(lp.allocate());
    new (so) SomeObject(); // LinkedPool does not construct the object
    so->setX(2);
    so->setY(5);
    assert(so->getX() == 2);
    assert(so->getY() == 5);
    assert(so->prod() == 10);

    auto so2 = lp.allocate();
    assert(so != so2);

    lp.deallocate(so);
    // note that LinkedPool does not free the memory allocated when it
    // goes out of scope, therefore `so2` is leaked
}
