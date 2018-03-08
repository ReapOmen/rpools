#include <cstdlib>
#include <new>

#include "rpools/allocators/NSGlobalLinkedPool.hpp"

using namespace rpools;

NSGlobalLinkedPool::NSGlobalLinkedPool(size_t t_sizeOfObjects,
                                       size_t t_alignment)
    : m_freePools(),
      m_sizeOfObjects(t_sizeOfObjects < sizeof(Node) ?
                      sizeof(Node) : t_sizeOfObjects),
      m_slotSize(m_sizeOfObjects) {
    avl_init(&m_freePools, nullptr);
    // make sure the first slot starts at a proper alignment
    size_t diff = mod(sizeof(PoolHeaderG), t_alignment);
    if (diff != 0) {
        m_headerPadding += t_alignment - diff;
    }
    // make sure that slots are properly aligned
    diff = mod(m_slotSize, t_alignment);
    if (diff != 0) {
        m_slotSize += t_alignment - diff;
    }
    m_poolSize = (getPageSize() - sizeof(PoolHeaderG) - m_headerPadding) /
        m_slotSize;
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
            Pool pool = aligned_alloc(getPageSize(), getPageSize());
            constructPoolHeader(reinterpret_cast<char*>(pool));
            pool_insert(&m_freePools, pool);
            m_freePool = pool;
            return nextFree(pool);
        }
    }
}

void NSGlobalLinkedPool::deallocate(void* t_ptr) {
    // get the pool of ptr
    auto pool = reinterpret_cast<PoolHeaderG*>(
        reinterpret_cast<size_t>(t_ptr) & getPoolMask()
    );
    if (pool->occupiedSlots == 1) {
        pool_remove(&m_freePools, pool);
        free(pool);
        m_freePool = pool_first(&m_freePools);
    } else {
        auto newNode = new (t_ptr) Node();
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
    // first slot after the header that is also aligned
    auto headNext = reinterpret_cast<Node*>(sizeof(PoolHeaderG) +
                                             t_ptr + m_headerPadding);
    // create the header at the start of the pool
    new (t_ptr) PoolHeaderG(m_sizeOfObjects, headNext);
    // skip the header
    t_ptr = reinterpret_cast<char*>(headNext);
    // for each slot in the pool, create a node that is linked to the next slot
    for (size_t i = 0; i < m_poolSize - 1; ++i) {
        auto newNode = new (t_ptr) Node();
        t_ptr += m_slotSize;
        newNode->next = reinterpret_cast<Node*>(t_ptr);
    }
    // create last node of the list, which isn't linked to anything
    new (t_ptr) Node();
}

void* NSGlobalLinkedPool::nextFree(Pool pool) {
    auto header = reinterpret_cast<PoolHeaderG*>(pool);
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
    size_t poolAddress = reinterpret_cast<size_t>(t_ptr) & getPoolMask();
    return *reinterpret_cast<PoolHeaderG*>(poolAddress);
}
