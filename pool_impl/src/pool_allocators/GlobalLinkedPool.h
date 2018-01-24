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
   Every pool will have a PoolHeader, which contains information
   about it.
   'sizeOfPool' denotes the number of slots that are occupied in the pool.
   'sizeOfObjects' denotes the size of objects that are stored in the pool.
   'head' denotes a Node which points to the first free slot.
 */
struct PoolHeaderG {
    size_t sizeOfPool;
    size_t sizeOfObjects;
    Node head;

    PoolHeaderG(size_t t_sizeOfObjects, Node* next)
        : sizeOfPool(0), sizeOfObjects(t_sizeOfObjects), head(next) {

    }

    bool operator ==(const PoolHeaderG& other) const {
        return sizeOfPool == other.sizeOfPool &&
            sizeOfObjects == other.sizeOfObjects &&
            head.next == other.head.next;
    }
};

/**
   GlobalLinkedPool is a pool allocation system which tries to minimise the amount
   of overheads created by allocating lots of objects on the heap.
   It works by allocating pools in chunks of PAGE_SIZE which makes deallocation
   very quick.
 */
class GlobalLinkedPool {
public:
    static const size_t PAGE_SIZE;
    // mask which is used to get the PoolHeader in constant time
    static const size_t POOL_MASK;

    /**
       Creates a GlobalLinkedPool allocator that will allocate objects of the
       given size in pools and return pointers to them.
     */
    GlobalLinkedPool();
    GlobalLinkedPool(size_t t_sizeOfObjects);

    /**
       Allocates space for an object of size N in one of the free slots
       and returns a pointer the mmeory location of where the object will be
       stored.
       @return A pointer to one of the free slots.
     */
    void* allocate();

    /**
       Deallocates the memory that is used by the object whose
       pointer is supplied.
       @param t_ptr - a pointer to an object that will be deallocated
     */
    void deallocate(void* t_ptr);

    size_t getPoolSize() const { return m_poolSize; }
    size_t getNumberOfPools() const { return pool_count(&m_freePools); }

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

    void constructPoolHeader(char* t_ptr);

    /**
       Returns a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool t_ptr);
};
}
#endif // __GLOBAL_LINKED_POOL_H__
