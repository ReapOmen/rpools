#ifndef __LINKED_POOL_3_H__
#define __LINKED_POOL_3_H__

#include <cstdlib>
#include <unistd.h>
#include <cmath>

extern "C" {
#include "avltree/avl_utils.h"
}

#ifdef __x86_64
#include "light_lock.h"
#else
#include <mutex>
#include <thread>
#endif

namespace efficient_pools3 {

using Pool = void*;

struct Node {
    Node* next;
};

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
class LinkedPool3 {
public:
    static const size_t PAGE_SIZE;
    // mask which is used to get the PoolHeader in constant time
    static const size_t POOL_MASK;

    /**
       Creates a LinkedPool allocator that will allocate objects of type T
       in pools and return pointers to them.
     */
    LinkedPool3();

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
#ifdef __x86_64
    light_lock_t m_poolLock;
#else
    std::mutex m_poolLock;
#endif
    const size_t m_poolSize;
    Pool m_freePool;

    void constructPoolHeader(Pool t_ptr);

    /**
       Returns a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool t_ptr);
};

template<typename T>
const size_t LinkedPool3<T>::PAGE_SIZE = sysconf(_SC_PAGESIZE);

template<typename T>
const size_t LinkedPool3<T>::POOL_MASK = -1 >> (size_t) std::log2(LinkedPool3::PAGE_SIZE)
                                        << (size_t) std::log2(LinkedPool3::PAGE_SIZE);

template<typename T>
LinkedPool3<T>::LinkedPool3()
    : m_freePools(),
      m_poolLock(
#ifdef __x86_64
          LIGHT_LOCK_INIT
#endif
      ),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeader)) / sizeof(T)),
      m_freePool(nullptr) {
}

template<typename T>
void* LinkedPool3<T>::allocate() {
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
            constructPoolHeader(pool);
            pool_insert(&m_freePools, pool);
            m_freePool = pool;
            return nextFree(pool);
        }
    }
}

template<typename T>
void LinkedPool3<T>::deallocate(void* t_ptr) {
    // get the pool of ptr
    PoolHeader* pool = reinterpret_cast<PoolHeader*>(
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
        Node* newNodeG = new (t_ptr) Node();
        // update nodes to point to the newly create Node
        Node& head = pool->head;
        newNodeG->next = head.next;
        head.next = newNodeG;
        m_freePool = pool;
        if (--pool->sizeOfPool == m_poolSize - 1) {
            pool_insert(&m_freePools, pool);
        }
    }
#ifdef __x86_64
    light_unlock(&m_poolLock);
#endif
}

template<typename T>
void LinkedPool3<T>::constructPoolHeader(Pool t_ptr) {
    PoolHeader* header = new (t_ptr) PoolHeader();
    T* first = reinterpret_cast<T*>(header + 1);
    header->head.next = reinterpret_cast<Node*>(first);
    for (size_t i = 0; i < m_poolSize - 1; ++i) {
        Node* node = new (first) Node();
        node->next = reinterpret_cast<Node*>(++first);
    }
    new (first) Node();
}

template<typename T>
void* LinkedPool3<T>::nextFree(Pool pool) {
    PoolHeader* header = reinterpret_cast<PoolHeader*>(pool);
    Node& head = header->head;
    void* toReturn = head.next;
    if (head.next) {
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

}
#endif // __LINKED_POOL_H__
