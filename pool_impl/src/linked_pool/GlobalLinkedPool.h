#ifndef __GLOBAL_LINKED_POOL_H__
#define __GLOBAL_LINKED_POOL_H__

#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <new>

extern "C" {
#include "avltree/avl_utils.h"
}


#ifdef __x86_64
#include "light_lock.h"
#else
#include <mutex>
#include <thread>
#endif

namespace efficient_pools {

using Pool = void*;

struct NodeG {
    NodeG* next;

    NodeG(NodeG* t_next = nullptr) : next(t_next) {}
};

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
    NodeG head;

    PoolHeaderG(size_t t_sizeOfObjects, NodeG* next)
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
    size_t getNumOfPools() const { return m_numOfPools; }

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
    size_t m_numOfPools;
    Pool m_freePool;

    void constructPoolHeader(char* t_ptr);

    /**
       Returns a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool t_ptr);
};

const size_t GlobalLinkedPool::PAGE_SIZE = sysconf(_SC_PAGESIZE);

const size_t GlobalLinkedPool::POOL_MASK = -1 >>
    (size_t) std::log2(GlobalLinkedPool::PAGE_SIZE)
    << (size_t) std::log2(GlobalLinkedPool::PAGE_SIZE);

GlobalLinkedPool::GlobalLinkedPool()
    : m_freePools(),
      m_poolLock(
#ifdef __x86_64
          LIGHT_LOCK_INIT
#endif
      ),
      m_sizeOfObjects(8),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeaderG)) / m_sizeOfObjects),
      m_numOfPools(0),
      m_freePool(nullptr) {
    avl_init(&m_freePools, NULL);
}

GlobalLinkedPool::GlobalLinkedPool(size_t t_sizeOfObjects)
    : m_freePools(),
      m_poolLock(
#ifdef __x86_64
          LIGHT_LOCK_INIT
#endif
      ),
      m_sizeOfObjects(t_sizeOfObjects < sizeof(NodeG) ? sizeof(NodeG) : t_sizeOfObjects),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeaderG)) / m_sizeOfObjects),
      m_numOfPools(0),
      m_freePool(nullptr) {
    avl_init(&m_freePools, NULL);
}

void* GlobalLinkedPool::allocate() {
#ifdef __x86_64
    light_lock(&m_poolLock);
#else
    std::lock_guard<std::mutex> lock(m_poolLock);
#endif
    if (m_freePool) {
        return nextFree(m_freePool);
    } else {
        Pool pool = pool_first(&m_freePools);
        if (pool) {
            return nextFree(pool);
        } else {
            // create a new pool because there are no free pool slots left
            Pool pool = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
            constructPoolHeader(reinterpret_cast<char*>(pool));
            pool_insert(&m_freePools, pool);
            m_freePool = pool;
            return nextFree(pool);
        }
    }
}

void GlobalLinkedPool::deallocate(void* t_ptr) {
    // get the pool of ptr
    PoolHeaderG* pool = reinterpret_cast<PoolHeaderG*>(
        reinterpret_cast<size_t>(t_ptr) & POOL_MASK
    );

#ifdef __x86_64
    light_lock(&m_poolLock);
#else
    std::lock_guard<std::mutex> lock(m_poolLock);
#endif

    if (pool->sizeOfPool == 1) {
        pool_remove(&m_freePools, pool);
        free(pool);
        m_freePool = pool_first(&m_freePools);
    } else {
        NodeG* newNodeG = new (t_ptr) NodeG();
        // update nodes to point to the newly create Node
        NodeG& head = pool->head;
        newNodeG->next = head.next;
        head.next = newNodeG;
        m_freePool = pool;
        if (--(pool->sizeOfPool) == m_poolSize - 1) {
            pool_insert(&m_freePools, pool);
        }
    }
#ifdef __x86_64
    light_unlock(&m_poolLock);
#endif
}

void GlobalLinkedPool::constructPoolHeader(char* t_ptr) {
    // first slot after the header
    NodeG* headNext = reinterpret_cast<NodeG*>(sizeof(PoolHeaderG) + t_ptr);
    // create the header at the start of the pool
    new (t_ptr) PoolHeaderG(m_sizeOfObjects, headNext);
    // skip the header
    t_ptr = reinterpret_cast<char*>(headNext);
    // for each slot in the pool, create a node that is linked to the next slot
    for (size_t i = 0; i < m_poolSize - 1; ++i) {
        NodeG* newNode = new (t_ptr) NodeG();
        t_ptr += m_sizeOfObjects;
        newNode->next = reinterpret_cast<NodeG*>(t_ptr);
    }
    // create last node of the list, which isn't linked to anything
    new (t_ptr) NodeG();
}

void* GlobalLinkedPool::nextFree(Pool pool) {
    PoolHeaderG* header = reinterpret_cast<PoolHeaderG*>(pool);
    NodeG& head = header->head;
    void* toReturn = head.next;
    if (toReturn) {
        head.next = head.next->next;
        if (++(header->sizeOfPool) == m_poolSize) {
            pool_remove(&m_freePools, pool);
            m_freePool = pool_first(&m_freePools);
        }
    }
#ifdef __x86_64
    light_unlock(&m_poolLock);
#endif
    return toReturn;
}

const PoolHeaderG& GlobalLinkedPool::getPoolHeader(void* t_ptr) {
    size_t poolAddress = reinterpret_cast<size_t>(t_ptr) & POOL_MASK;
    return *reinterpret_cast<PoolHeaderG*>(poolAddress);
}

}
#endif // __GLOBAL_LINKED_POOL_H__
