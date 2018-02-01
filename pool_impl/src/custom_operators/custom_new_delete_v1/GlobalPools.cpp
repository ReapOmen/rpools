#include "GlobalPools.h"

#include <cstddef>
#include <cmath>

using std::vector;
using efficient_pools::NSGlobalLinkedPool;

const size_t __void = sizeof(void*);
const size_t __logOfVoid = std::log2(__void);

GlobalPools::GlobalPools(size_t t_numOfPools)
    : m_pools() {
    m_pools.reserve(t_numOfPools);
    for (size_t i = __void; i <= t_numOfPools * __void; i += __void) {
        size_t alignment = (i & (alignof(max_align_t) - 1)) == 0 ?
            alignof(max_align_t) : __void;
        m_pools.push_back(NSGlobalLinkedPool(i, alignment));
    }
}

NSGlobalLinkedPool& GlobalPools::getPool(size_t t_size) {
    if (t_size == 0) {
        return m_pools[0];
    }
    return m_pools[(t_size >> __logOfVoid) - 1];
}
