#ifndef __NS_GLOBAL_LINKED_POOL_H__
#define __NS_GLOBAL_LINKED_POOL_H__

#include "Node.h"

extern "C" {
#include "avltree/avl_utils.h"
}

namespace efficient_pools {

using Pool = void*;

struct PoolHeaderG {
    size_t occupiedSlots;
    size_t sizeOfSlot;
    Node head;

    PoolHeaderG(size_t t_sizeOfSlot, Node* t_next)
        : occupiedSlots(0), sizeOfSlot(t_sizeOfSlot), head(t_next) {

    }

    bool operator ==(const PoolHeaderG& other) const {
        return occupiedSlots == other.occupiedSlots &&
            sizeOfSlot == other.sizeOfSlot &&
            head.next == other.head.next;
    }
};

/**
 *  A thread unsafe version of `GlobalLinkedPool`.
 *  @see GlobalLinkedPool
 */
class NSGlobalLinkedPool {
public:
    static const size_t PAGE_SIZE;
    static const size_t POOL_MASK;

    NSGlobalLinkedPool();
    NSGlobalLinkedPool(size_t t_sizeOfObjects);

    void* allocate();
    void deallocate(void* t_ptr);
    size_t getPoolSize() const { return m_poolSize; }
    size_t getNumberOfPools() const { return pool_count(&m_freePools); }
    static const PoolHeaderG& getPoolHeader(void* t_ptr);

private:
    avl_tree m_freePools;
    const size_t m_sizeOfObjects;
    const size_t m_poolSize;
    Pool m_freePool;

    void constructPoolHeader(char* t_ptr);
    void* nextFree(Pool t_ptr);
};
}
#endif // __NS_GLOBAL_LINKED_POOL_H__
