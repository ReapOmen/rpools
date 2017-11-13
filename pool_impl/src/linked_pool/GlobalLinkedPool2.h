#ifndef __LINKED_POOL_H__
#define __LINKED_POOL_H__

#include <cstdlib>
#include <unistd.h>
#include <unordered_set>
#include <cmath>

#include "../CustomAlloc.h"

namespace efficient_pools {

using Pool = void*;

struct Node {
    Node* next;
};

/**
   Every pool will have a PoolHeader, which contains information
   about it.
   'sizeOfPool' denotes the number of slots that are occupied in the pool.
   'sizeOfObjects' denotes the size of objects that are stored in the pool.
   'head' denotes a Node which points to the first free slot.
 */
struct PoolHeader {
    size_t sizeOfPool;
    size_t sizeOfObjects;
    Node head;
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

    size_t getPoolSize() { return m_poolSize; }

    static const PoolHeader& getPoolHeader(void* t_ptr);

private:
    const size_t m_sizeOfObjects;
    const size_t m_poolSize;
    Pool m_freePool;
    std::unordered_set<Pool, std::less<Pool>, mallocator<Pool>> m_freePools;

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
    : m_sizeOfObjects(1),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeader)) / m_sizeOfObjects),
      m_freePool(nullptr),
      m_freePools() {
}

GlobalLinkedPool::GlobalLinkedPool(size_t t_sizeOfObjects)
    : m_sizeOfObjects(t_sizeOfObjects < 8 ? 8 : t_sizeOfObjects),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeader)) / m_sizeOfObjects),
      m_freePool(nullptr),
      m_freePools() {
}

void* GlobalLinkedPool::allocate() {
    if (m_freePool) {
        return nextFree(m_freePool);
    } else if (m_freePools.size() > 0) {
        return nextFree(*m_freePools.begin());
    } else {
        // create a new pool because there are no free pool slots left
        Pool pool = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        // init all the metadata
        PoolHeader* header = reinterpret_cast<PoolHeader*> (pool);
        *header = PoolHeader();
        header->sizeOfObjects = m_sizeOfObjects;
        header->head.next = reinterpret_cast<Node*>(header + 1);
        char* first = reinterpret_cast<char*>(header + 1);
        for (int i = 0; i < m_poolSize - 1; ++i) {
            Node* node = reinterpret_cast<Node*>(first);
            *node = Node();
            first += m_sizeOfObjects;
            node->next = reinterpret_cast<Node*>(first);
        }
        Node* node = reinterpret_cast<Node*>(first);
        *node = Node();
        m_freePools.insert(pool);
        m_freePool = pool;
        return nextFree(pool);
    }
}

void GlobalLinkedPool::deallocate(void* t_ptr) {
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
        m_freePool = m_freePools.size() == 0 ? nullptr : *m_freePools.begin();
    } else {
        m_freePool = pool;
        if (newSize == m_poolSize - 1) {
            m_freePools.insert(pool);
        }
    }
}

void* GlobalLinkedPool::nextFree(Pool pool) {
    PoolHeader* header = reinterpret_cast<PoolHeader*>(pool);
    Node* head = &(header->head);
    if (head->next) {
        void* toReturn = head->next;
        head->next = head->next->next;
        if (++(header->sizeOfPool) == m_poolSize) {
            m_freePools.erase(pool);
            m_freePool = m_freePools.size() == 0 ? nullptr : *m_freePools.begin();
        }
        return toReturn;
    }
    return head->next;
}

const PoolHeader& GlobalLinkedPool::getPoolHeader(void* t_ptr) {
    size_t poolAddress = reinterpret_cast<size_t>(t_ptr) & POOL_MASK;
    return *reinterpret_cast<PoolHeader*>(poolAddress);
}

}
#endif // __LINKED_POOL_H__
