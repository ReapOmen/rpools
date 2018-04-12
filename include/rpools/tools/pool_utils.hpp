#ifndef __POOL_UTILS_H__
#define __POOL_UTILS_H__

#include <cstddef>
#include <unistd.h>
#include <cmath>

namespace rpools {

/** 
 *  @return the page size of the system. 
 */
inline long int getPageSize() {
    static long int pageSize = sysconf(_SC_PAGESIZE);
    return pageSize;
}

/** 
 *  Mask which is used to get the `PoolHeader` in constant time.
 *  Because `PoolHeader`s are page aligned, masking a pointer that is
 *  allocated in a pool will give the address of the pool's `PoolHeader`.
 */
inline size_t getPoolMask() {
    static size_t poolMask = ~0 >> (size_t) std::log2(getPageSize())
				<< (size_t) std::log2(getPageSize());
    return poolMask;
}

/**
 *  @param t_l the lhs of the `%` operator
 *  @param t_powOfTwo a power of 2 which is also the rhs of the `%` operator
 *  @return `t_l % t_powOfTwo`.
 */
inline size_t mod(size_t t_l, size_t t_powOfTwo) {
    return t_l & (t_powOfTwo - 1);
}

};

#endif
