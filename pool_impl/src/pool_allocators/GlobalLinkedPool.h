#ifndef __GLOBAL_LINKED_POOL_H__
#define __GLOBAL_LINKED_POOL_H__

#include "Node.h"

extern "C" {
#include "avltree/avl_utils.h"
}

#ifdef __x86_64
#include "tools/light_lock.h"
#else
#include <mutex>
#include <thread>
#endif

namespace efficient_pools {

using Pool = void*;

/**
 *  Every pool is allocated on a page boundary.
 *  The `PoolHeader` is placed at the first byte of the page and
 *  contains certain metadata about a pool.
 */
struct PoolHeaderG {
    /** Denotes the number of slots that are occupied. */
    size_t occupiedSlots;
    /** The size of a pool slot. */
    size_t sizeOfSlot;
     /** A `Node` which points to the next free slot of the pool, or
      *  to nullptr if there are no slots left. */
    Node head;

    /**
     *  Create a `PoolHeaderG` with non-default values.
     *  @param t_sizeOfSlot the size of a slot in the pool
     *  @param t_next the first empty pool slot
     */
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
     *  Creates a `GlobalLinkedPool` allocator that will allocate objects of
     *  size **8** in pools and return pointers to them.
     *  @param t_sizeOfObjects the maximum size that a pool slot will have
     */
    GlobalLinkedPool();
    /**
     *  Creates a `GlobalLinkedPool` allocator that will allocate objects of the
     *  given size in pools and return pointers to them.
     *  @param t_sizeOfObjects the maximum size that a pool slot will have
     */
    GlobalLinkedPool(size_t t_sizeOfObjects);

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
#ifdef __x86_64
    light_lock_t m_poolLock;
#else
    std::mutex m_poolLock;
#endif
    const size_t m_sizeOfObjects;
    const size_t m_poolSize;
    Pool m_freePool;

    /** @see LinkedPool3::constructPoolHeader */
    void constructPoolHeader(char* t_ptr);

    /**
     *  @return a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool t_ptr);
};
}
#endif // __GLOBAL_LINKED_POOL_H__
