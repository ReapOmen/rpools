#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <new>

#include "NSGlobalLinkedPool.h"

using namespace efficient_pools;

const size_t NSGlobalLinkedPool::PAGE_SIZE = sysconf(_SC_PAGESIZE);

const size_t NSGlobalLinkedPool::POOL_MASK = ~0 >>
    (size_t) std::log2(NSGlobalLinkedPool::PAGE_SIZE)
    << (size_t) std::log2(NSGlobalLinkedPool::PAGE_SIZE);

NSGlobalLinkedPool::NSGlobalLinkedPool()
    : m_freePools(),
      m_sizeOfObjects(8),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeaderG)) / m_sizeOfObjects),
      m_freePool(nullptr) {
    avl_init(&m_freePools, NULL);
}

NSGlobalLinkedPool::NSGlobalLinkedPool(size_t t_sizeOfObjects)
    : m_freePools(),
      m_sizeOfObjects(t_sizeOfObjects < sizeof(Node) ? sizeof(Node) : t_sizeOfObjects),
      m_poolSize((PAGE_SIZE - sizeof(PoolHeaderG)) / m_sizeOfObjects),
      m_freePool(nullptr) {
    avl_init(&m_freePools, NULL);
}

void* NSGlobalLinkedPool::allocate() {
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

void NSGlobalLinkedPool::deallocate(void* t_ptr) {
    // get the pool of ptr
    PoolHeaderG* pool = reinterpret_cast<PoolHeaderG*>(
        reinterpret_cast<size_t>(t_ptr) & POOL_MASK
    );
    if (pool->occupiedSlots == 1) {
        pool_remove(&m_freePools, pool);
        free(pool);
        m_freePool = pool_first(&m_freePools);
    } else {
        Node* newNode = new (t_ptr) Node();
        // update nodes to point to the newly create Node
        Node& head = pool->head;
        newNode->next = head.next;
        head.next = newNode;
        m_freePool = pool;
        if (--(pool->occupiedSlots) == m_poolSize - 1) {
            pool_insert(&m_freePools, pool);
        }
    }
}

void NSGlobalLinkedPool::constructPoolHeader(char* t_ptr) {
    // first slot after the header
    Node* headNext = reinterpret_cast<Node*>(sizeof(PoolHeaderG) + t_ptr);
    // create the header at the start of the pool
    new (t_ptr) PoolHeaderG(m_sizeOfObjects, headNext);
    // skip the header
    t_ptr = reinterpret_cast<char*>(headNext);
    // for each slot in the pool, create a node that is linked to the next slot
    for (size_t i = 0; i < m_poolSize - 1; ++i) {
        Node* newNode = new (t_ptr) Node();
        t_ptr += m_sizeOfObjects;
        newNode->next = reinterpret_cast<Node*>(t_ptr);
    }
    // create last node of the list, which isn't linked to anything
    new (t_ptr) Node();
}

void* NSGlobalLinkedPool::nextFree(Pool pool) {
    PoolHeaderG* header = reinterpret_cast<PoolHeaderG*>(pool);
    Node& head = header->head;
    void* toReturn = head.next;
    if (toReturn) {
        head.next = head.next->next;
        if (++(header->occupiedSlots) == m_poolSize) {
            pool_remove(&m_freePools, pool);
            m_freePool = pool_first(&m_freePools);
        }
    }
    return toReturn;
}

const PoolHeaderG& NSGlobalLinkedPool::getPoolHeader(void* t_ptr) {
    size_t poolAddress = reinterpret_cast<size_t>(t_ptr) & POOL_MASK;
    return *reinterpret_cast<PoolHeaderG*>(poolAddress);
}
