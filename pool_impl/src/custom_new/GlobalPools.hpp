#ifndef __GLOBAL_POOLS_H__
#define __GLOBAL_POOLS_H__

#include <vector>

#include "rpools/tools/mallocator.hpp"
#include "rpools/allocators/GlobalLinkedPool.hpp"

/**
 *  Represents a class which holds `GlobalLinkedPool`s that can
 *  hold objects that are multiples of 8.
 *  @par
 *  Example: GlobalPools(8) will create 8 `GlobalLinkedPool`s.
 *  The first pool can hold objects of sizes up to 8 and will align them
 *  at 8 byte boundaries. The 2nd pool will hold objects of size 16, but will
 *  align them at 16 byte boundaries, and so on.
 */
class GlobalPools {
public:
    /**
     *  Allocates `t_numOfPools` pools.
     *  @param t_numOfPools the number of `GlobalLinkedPool`s to allocate
     */
    GlobalPools(size_t t_numOfPools);

    /**
     *  Gets the `GlobalLinkedPool` that can hold objects of sizes up to
     *  `t_size * 8`.
     */
    rpools::GlobalLinkedPool& getPool(size_t t_size);
    virtual ~GlobalPools() = default;
private:
    std::vector<rpools::GlobalLinkedPool,
                mallocator<rpools::GlobalLinkedPool>
    > m_pools;
};

#endif // __GLOBAL_POOLS_H__
