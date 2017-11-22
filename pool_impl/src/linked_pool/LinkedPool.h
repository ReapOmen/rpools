#ifndef __LINKED_POOL_H__
#define __LINKED_POOL_H__

#include <cstdlib>
#include <unistd.h>
#include <unordered_set>
#include <cmath>

namespace efficient_pools {

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

    const size_t m_poolSize;
    std::unordered_set<Pool> m_freePools;

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
    : m_poolSize((PAGE_SIZE - sizeof(PoolHeader)) / sizeof(T)),
      m_freePools() {
}

template<typename T>
void* LinkedPool<T>::allocate() {
    if (m_freePools.size() > 0) {
         return nextFree(*m_freePools.begin());
    } else {
        // create a new pool because there are no free pool slots left
        Pool pool = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        // init all the metadata
        PoolHeader* header = reinterpret_cast<PoolHeader*> (pool);
        *header = PoolHeader();
        header->head.next = reinterpret_cast<Node*>(header + 1);
        T* first = reinterpret_cast<T*>(header + 1);
        for (size_t i = 0; i < m_poolSize - 1; ++i) {
            Node* node = reinterpret_cast<Node*>(first);
            *node = Node();
            node->next = reinterpret_cast<Node*>(++first);
        }
        Node* node = reinterpret_cast<Node*>(first);
        *node = Node();
        m_freePools.insert(pool);
        return nextFree(pool);
    }
}

template<typename T>
void LinkedPool<T>::deallocate(void* t_ptr) {
    Node* newNode = reinterpret_cast<Node*>(t_ptr);
    *newNode = Node();
    // get the pool of ptr
    PoolHeader* pool = reinterpret_cast<PoolHeader*>(
        reinterpret_cast<size_t>(t_ptr) & POOL_MASK
    );
    // update nodes to point to the newly create Node
    Node* head = &pool->head;
    if (head->next == nullptr) {
        head->next = newNode;
    } else {
        newNode->next = head->next;
        head->next = newNode;
    }
    // pool is empty, let's free it!
    size_t newSize = --pool->sizeOfPool;
    if (newSize == 0) {
        m_freePools.erase(pool);
        free(pool);
    } else {
        if (newSize == m_poolSize - 1) {
            m_freePools.insert(pool);
        }
    }
}

template<typename T>
void* LinkedPool<T>::nextFree(Pool pool) {
    PoolHeader* header = reinterpret_cast<PoolHeader*>(pool);
    Node* head = &(header->head);
    if (head->next) {
        void* toReturn = head->next;
        head->next = head->next->next;
        if (++(header->sizeOfPool) == m_poolSize) {
            m_freePools.erase(pool);
        }
        return toReturn;
    }
    return head->next;
}

}
#endif // __LINKED_POOL_H__
