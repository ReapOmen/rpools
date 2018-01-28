#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <new>

#include "GlobalLinkedPool.h"

using namespace efficient_pools;

const size_t GlobalLinkedPool::PAGE_SIZE = sysconf(_SC_PAGESIZE);

const size_t GlobalLinkedPool::POOL_MASK = ~0 >>
    (size_t) std::log2(GlobalLinkedPool::PAGE_SIZE)
    << (size_t) std::log2(GlobalLinkedPool::PAGE_SIZE);

GlobalLinkedPool::GlobalLinkedPool(size_t t_sizeOfObjects,
                                   size_t t_alignment)
    : m_freePools(),
      m_poolLock(
#ifdef __x86_64
          LIGHT_LOCK_INIT
#endif
      ),
      m_sizeOfObjects(t_sizeOfObjects < sizeof(Node) ?
                      sizeof(Node) : t_sizeOfObjects),
      m_headerPadding(0),
      m_slotSize(m_sizeOfObjects),
      m_poolSize(0),
      m_freePool(nullptr) {
    avl_init(&m_freePools, NULL);
    // make sure the first slot starts at a proper alignment
    size_t diff = sizeof(PoolHeaderG) % t_alignment;
    if (diff != 0) {
        m_headerPadding += t_alignment - diff;
    }
    // make sure that slots are properly aligned
    diff = m_slotSize % t_alignment;
    if (diff != 0) {
        m_slotSize += t_alignment - diff;
    }
    m_poolSize = (PAGE_SIZE - sizeof(PoolHeaderG) - m_headerPadding) /
        m_slotSize;
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
#ifdef __x86_64
    light_unlock(&m_poolLock);
#endif
}

void GlobalLinkedPool::constructPoolHeader(char* t_ptr) {
    // first slot after the header that is also aligned
    Node* headNext = reinterpret_cast<Node*>(sizeof(PoolHeaderG) +
                                             t_ptr + m_headerPadding);
    // create the header at the start of the pool
    new (t_ptr) PoolHeaderG(m_sizeOfObjects, headNext);
    // skip the header
    t_ptr = reinterpret_cast<char*>(headNext);
    // for each slot in the pool, create a node that is linked to the next slot
    for (size_t i = 0; i < m_poolSize - 1; ++i) {
        Node* newNode = new (t_ptr) Node();
        t_ptr += m_slotSize;
        newNode->next = reinterpret_cast<Node*>(t_ptr);
    }
    // create last node of the list, which isn't linked to anything
    new (t_ptr) Node();
}

void* GlobalLinkedPool::nextFree(Pool pool) {
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
#ifdef __x86_64
    light_unlock(&m_poolLock);
#endif
    return toReturn;
}

const PoolHeaderG& GlobalLinkedPool::getPoolHeader(void* t_ptr) {
    size_t poolAddress = reinterpret_cast<size_t>(t_ptr) & POOL_MASK;
    return *reinterpret_cast<PoolHeaderG*>(poolAddress);
}
