#ifndef __NS_GLOBAL_LINKED_POOL_H__
#define __NS_GLOBAL_LINKED_POOL_H__

#include "pool_allocators/PoolHeaderG.hpp"

extern "C" {
#include "avltree/avl_utils.h"
}

namespace rpools {

using Pool = void*;

/**
 *  A thread unsafe version of `GlobalLinkedPool`.
 *  @see GlobalLinkedPool
 */
class NSGlobalLinkedPool {
public:
    static const size_t PAGE_SIZE;
    static const size_t POOL_MASK;

    NSGlobalLinkedPool(size_t t_sizeOfObjects=sizeof(Node),
                       size_t alignment=alignof(max_align_t));

    void* allocate();
    void deallocate(void* t_ptr);
    size_t getPoolSize() const { return m_poolSize; }
    size_t getNumberOfPools() const { return pool_count(&m_freePools); }
    static const PoolHeaderG& getPoolHeader(void* t_ptr);

private:
    avl_tree m_freePools;
    const size_t m_sizeOfObjects;
    size_t m_headerPadding = 0;
    size_t m_slotSize;
    size_t m_poolSize = 0;
    Pool m_freePool = nullptr;

    void constructPoolHeader(char* t_ptr);
    void* nextFree(Pool t_ptr);
};
}

#endif // __NS_GLOBAL_LINKED_POOL_H__
