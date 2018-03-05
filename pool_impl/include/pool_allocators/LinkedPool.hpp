#ifndef __LINKED_POOL_H__
#define __LINKED_POOL_H__

#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <new>

#include "pool_allocators/Node.hpp"
#include "tools/LMLock.hpp"

extern "C" {
#include "avltree/avl_utils.h"
}

namespace efficient_pools {

using Pool = void*;

/**
   Every pool will have a PoolHeader, which contains information
   about it. 'sizeOfPool' denotes the number of slots that are
   occupied in the pool. 'head' denotes a Node which points to
   the first free slot.
 */
struct PoolHeader {
    size_t sizeOfPool;
    Node head;
};

/**
   LinkedPool is a pool allocation system which tries to minimise the amount
   of overheads created by allocating lots of objects on the heap.
   It works by allocating pools in chunks of PAGE_SIZE which makes deallocation
   very quick.
 */
template<typename T>
class LinkedPool {
public:
    static const size_t PAGE_SIZE;
    // mask which is used to get the PoolHeader in constant time
    static const size_t POOL_MASK;

    /**
       Creates a LinkedPool allocator that will allocate objects of type T
       in pools and return pointers to them.
     */
    LinkedPool();

    /**
       Allocates space for an object of type T in one of the free slots
       and returns a pointer the mmeory location of where the object will be
       stored.
       @return A pointer to the newly allocated space for T.
     */
    void* allocate();

    /**
       Deallocates the memory that is used by the object of type T whose
       pointer is supplied.
       @param t_ptr - a pointer to an object that will be deallocated
     */
    void deallocate(void* t_ptr);

    size_t getPoolSize() { return m_poolSize; }

private:
    avl_tree m_freePools;
    LMLock m_poolLock;
    const size_t m_poolSize;

    void constructPoolHeader(Pool t_ptr);

    /**
       Returns a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool t_ptr);
};

template<typename T>
const size_t LinkedPool<T>::PAGE_SIZE = sysconf(_SC_PAGESIZE);

template<typename T>
const size_t LinkedPool<T>::POOL_MASK = -1 >> (size_t) std::log2(LinkedPool::PAGE_SIZE)
                                        << (size_t) std::log2(LinkedPool::PAGE_SIZE);

template<typename T>
LinkedPool<T>::LinkedPool()
    : m_freePools(),
      m_poolLock(),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeader)) / sizeof(T)) {
    avl_init(&m_freePools, nullptr);
}

template<typename T>
void* LinkedPool<T>::allocate() {
    m_poolLock.lock();
    Pool freePool = pool_first(&m_freePools);
    if (freePool) {
        return nextFree(freePool);
    } else {
        // create a new pool because there are no free pool slots left
        Pool pool = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        constructPoolHeader(pool);
        pool_insert(&m_freePools, pool);
        return nextFree(pool);
    }
}

template<typename T>
void LinkedPool<T>::deallocate(void* t_ptr) {
    // get the pool of ptr
    auto pool = reinterpret_cast<PoolHeader*>(
        reinterpret_cast<size_t>(t_ptr) & POOL_MASK
    );
    m_poolLock.lock();
    if (pool->sizeOfPool == 1) {
        pool_remove(&m_freePools, pool);
        free(pool);
    } else {
        auto newNode = new (t_ptr) Node();
        // update nodes to point to the newly create Node
        Node& head = pool->head;
        newNode->next = head.next;
        head.next = newNode;
        if (--pool->sizeOfPool == m_poolSize - 1) {
            pool_insert(&m_freePools, pool);
        }
    }
    m_poolLock.unlock();
}

template<typename T>
void LinkedPool<T>::constructPoolHeader(Pool t_ptr) {
    PoolHeader* header = new (t_ptr) PoolHeader();
    auto first = reinterpret_cast<T*>(header + 1);
    header->head.next = reinterpret_cast<Node*>(first);
    for (size_t i = 0; i < m_poolSize - 1; ++i) {
        auto node = new (first) Node();
        node->next = reinterpret_cast<Node*>(++first);
    }
    new (first) Node();
}

template<typename T>
void* LinkedPool<T>::nextFree(Pool t_ptr) {
    auto header = reinterpret_cast<PoolHeader*>(t_ptr);
    Node& head = header->head;
    void* toReturn = head.next;
    if (head.next) {
        head.next = head.next->next;
        if (++(header->sizeOfPool) == m_poolSize) {
            pool_remove(&m_freePools, t_ptr);
        }
    }
    m_poolLock.unlock();
    return toReturn;
}

}
#endif // __LINKED_POOL_H__
