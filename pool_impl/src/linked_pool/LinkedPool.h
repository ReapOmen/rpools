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

struct PoolHeader {
    size_t sizeOfPool;
    Node head;
};

/**
   LinkedPool is a pool allocation system which tries to minimise the amount
   of overheads created by allocating lots of objects on the heap.
 */
template<typename T>
class LinkedPool {
public:

    /**
       Creates a LinkedPool allocator that will allocate objects of type T
       in pools and return pointers to them.
     */
    LinkedPool();

    /**
       Allocates an object in one of the free slots and returns a pointer
       to it. The object's default constructor will be called.
       @return A pointer to the newly created object of type T.
     */
    void* allocate();

    /**
       Deallocates the given pointer that was allocated using the allocate
       method.
       @param t_ptr - a pointer to an object that will be deallocated
     */
    void deallocate(void* t_ptr);

private:

    static const size_t PAGE_SIZE;
    static const size_t POOL_MASK;

    const size_t m_poolSize;
    Pool m_freePool;
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
      m_freePool(nullptr),
      m_freePools() {
}

template<typename T>
void* LinkedPool<T>::allocate() {
    if (m_freePool) {
        return nextFree(m_freePool);
    } else if (m_freePools.size() > 0) {
        m_freePool = *m_freePools.begin();
         return nextFree(m_freePool);
    } else {
        // create a new pool because there are no free pool slots left
        Pool pool = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        // init all the metadata
        PoolHeader* header = reinterpret_cast<PoolHeader*> (pool);
        *header = PoolHeader();
        header->head.next = reinterpret_cast<Node*>(header + 1);
        T* first = reinterpret_cast<T*>(header + 1);
        for (int i = 0; i < m_poolSize - 1; ++i) {
            Node* node = reinterpret_cast<Node*>(first);
            *node = Node();
            node->next = reinterpret_cast<Node*>(++first);
        }
        Node* node = reinterpret_cast<Node*>(first);
        *node = Node();
        m_freePools.insert(pool);
        m_freePool = pool;
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
        while (t_ptr < head) {
            head = head->next;
        }
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
            m_freePools.insert(m_freePool);
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
            m_freePool = m_freePools.size() == 0 ? nullptr : *m_freePools.begin();
        }
        return toReturn;
    }
    return head->next;
}

}
#endif // __LINKED_POOL_H__
