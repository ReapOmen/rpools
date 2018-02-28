#ifndef __GLOBAL_POOLS_H__
#define __GLOBAL_POOLS_H__

#include <vector>

#include "tools/mallocator.hpp"
#include "pool_allocators/NSGlobalLinkedPool.hpp"

class GlobalPools {
public:
    GlobalPools(size_t t_numOfPools);
    efficient_pools::NSGlobalLinkedPool& getPool(size_t t_size);
    virtual ~GlobalPools() = default;
private:
    std::vector<efficient_pools::NSGlobalLinkedPool,
                mallocator<efficient_pools::NSGlobalLinkedPool>
    > m_pools;
};

#endif // __GLOBAL_POOLS_H__
