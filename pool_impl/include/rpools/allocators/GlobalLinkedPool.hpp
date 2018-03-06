#ifndef __GLOBAL_LINKED_POOL_H__
#define __GLOBAL_LINKED_POOL_H__

extern "C" {
#include "rpools/avltree/avl_utils.h"
}
#include "rpools/allocators/PoolHeaderG.hpp"
#include "rpools/tools/LMLock.hpp"

namespace rpools {

using Pool = void*;

/**
 *  Represents a pool allocator which tries to minimise the amount of memory
 *  overheads of small objects.
 *  This is done by allocating a number of pages of memory in which objects
 *  will be allocated.
 *  @par
 *  A `GlobalLinkedPool` does not know the type of object, but will be able
 *  to allocate objects that have the same size.
 *  @note When all objects of a page are deallocated, the page is freed.
 */
class GlobalLinkedPool {
public:
    /** The page size of the system. */
    static const size_t PAGE_SIZE;
    /** Mask which is used to get the `PoolHeader` in constant time.
     *  Because `PoolHeader`s are page aligned, masking a pointer that is
     *  allocated in a pool will give the address of the pool's `PoolHeader`.
     */
    static const size_t POOL_MASK;

    /**
     *  Creates a `GlobalLinkedPool` allocator that will allocate objects of the
     *  given size in pools and return pointers to them.
     *  @param t_sizeOfObjects the maximum size that a pool slot will have
     *                         (default: sizeof(Node))
     *  @param t_alignment the alignment of the objects
     *                     (default: alignof(max_align_t))
     */
    GlobalLinkedPool(size_t t_sizeOfObjects=sizeof(Node),
                     size_t t_alignment=alignof(max_align_t));

    /**
     *  Allocates space for an object of size N in one of the free slots
     *  and returns a pointer the memory location of where the object will be
     *  stored.
     *  @return A pointer to one of the free slots.
     */
    void* allocate();

    /**
     *  Deallocates the memory that is used by the object whose
     *  pointer is supplied.
     *  @param t_ptr - a pointer to an object that will be deallocated
     */
    void deallocate(void* t_ptr);

    /**
     *  @return the number of slots that fit in a page of memory.
     */
    size_t getPoolSize() const { return m_poolSize; }

    /**
     *  @warning this is not a constant time operation so use it wisely!
     *  @return the number of pages that are currently allocated
     */
    size_t getNumberOfPools() const { return pool_count(&m_freePools); }

    /**
     *  @param t_ptr the pointer of a slot of the pool
     *  @return the `PoolHeaderG` at **t_ptr & PAGE_MASK**.
     */
    static const PoolHeaderG& getPoolHeader(void* t_ptr);

private:
    avl_tree m_freePools;
    LMLock m_poolLock;
    const size_t m_sizeOfObjects;
    size_t m_headerPadding = 0;
    size_t m_slotSize;
    size_t m_poolSize = 0;
    Pool m_freePool = nullptr;

    /** @see LinkedPool3::constructPoolHeader */
    void constructPoolHeader(char* t_ptr);

    /**
     *  @return a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool t_ptr);
};
}
#endif // __GLOBAL_LINKED_POOL_H__
