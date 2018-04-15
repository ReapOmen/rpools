#include <memory>
#include <cassert>
#include "rpools/allocators/LinkedPool.hpp"
using rpools::LinkedPool;

class SomeObject {
public:
    void setX(int t_x) { m_x = t_x; }
    void setY(int t_y) { m_y = t_y; }
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int prod() const { return m_x * m_y; }
    static void* operator new(std::size_t t_size) {
        // note that `allocate` does not throw, but to keep
        // the example simple, checking for nullptr is ommited
        return lp.allocate();
    }

    static void operator delete(void* t_ptr) {
        lp.deallocate(t_ptr);
    }
private:
    static LinkedPool<SomeObject> lp;
    int m_x, m_y;
};

LinkedPool<SomeObject> SomeObject::lp;

int main() {
    auto so = new SomeObject();
    so->setX(2);
    so->setY(5);
    assert(so->getX() == 2);
    assert(so->getY() == 5);
    assert(so->prod() == 10);

    auto so2 = std::unique_ptr<SomeObject>(new SomeObject());
    assert(so != so2.get());
    delete so;
}
