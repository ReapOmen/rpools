#ifndef __GLOBAL_POOLS_H__
#define __GLOBAL_POOLS_H__

#include <vector>

#include "tools/mallocator.hpp"
#include "pool_allocators/NSGlobalLinkedPool.hpp"

/**
 *  Represents a class which holds `NSGlobalLinkedPool`s that can
 *  hold objects that are multiples of 8.
 *  @par
 *  Example: GlobalPools(8) will create 8 `NSGlobalLinkedPool`s.
 *  The first pool can hold objects of sizes up to 8 and will align them
 *  at 8 byte boundaries. The 2nd pool will hold objects of size 16, but will
 *  align them at 16 byte boundaries, and so on.
 */
class GlobalPools {
public:
    /**
     *  Allocates `t_numOfPools` pools.
     *  @param t_numOfPools the number of `NSGlobalLinkedPool`s to allocate
     */
    GlobalPools(size_t t_numOfPools);

    /**
     *  Gets the `NSGlobalLinkedPool` that can hold objects of sizes up to
     *  `t_size * 8`.
     */
    efficient_pools::NSGlobalLinkedPool& getPool(size_t t_size);
    virtual ~GlobalPools() = default;
private:
    std::vector<efficient_pools::NSGlobalLinkedPool,
                mallocator<efficient_pools::NSGlobalLinkedPool>
    > m_pools;
};

#endif // __GLOBAL_POOLS_H__
