#ifndef __POOL_HEADER_G__
#define __POOL_HEADER_G__

#include <cstddef>
#include "rpools/allocators/Node.hpp"

namespace rpools {

/**
 *  Every pool is allocated on a page boundary.
 *  The `PoolHeaderG` is placed at the first byte of the page and
 *  contains certain metadata about a pool.
 */
struct PoolHeaderG {
    /** Denotes the number of slots that are occupied. */
    size_t occupiedSlots;
    /** The size of a pool slot. */
    size_t sizeOfSlot;
     /** A `Node` which points to the next free slot of the pool, or
      *  to nullptr if there are no slots left. */
    Node head;

    /**
     *  Create a `PoolHeaderG` with non-default values.
     *  @param t_sizeOfSlot the size of a slot in the pool
     *  @param t_next the first empty pool slot
     */
    PoolHeaderG(size_t t_sizeOfSlot, Node* t_next)
        : occupiedSlots(0), sizeOfSlot(t_sizeOfSlot), head(t_next) {

    }

    bool operator ==(const PoolHeaderG& other) const {
        return occupiedSlots == other.occupiedSlots &&
            sizeOfSlot == other.sizeOfSlot &&
            head.next == other.head.next;
    }
};
}

#endif // __POOL_HEADER_G__
