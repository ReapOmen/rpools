#ifndef __BIT_POOL_H__
#define __BIT_POOL_H__

#include <vector>
#include <cstdlib>
#include <algorithm>

using Pool = void*;

/**
   Used by getPool.
 */
struct Pair {
    Pool pool;
    size_t bits;

    Pair(Pool poolIn, size_t bitsIn);
};

/**
   BitPool is a pool allocation system which tries to minimise the amount
   of overheads created by allocating lots of objects on the heap.
 */
template<typename T>
class BitPool {
public:

    /**
       Creates a BitPool allocator that will create pools of the given size.
       By default, poolSize is 80.
       @param poolSize - the size of the pools that will be
                         allocated (80 by default)
     */
    BitPool(size_t poolSize = 80);

    ~BitPool();

    /**
       Allocates an object in one of the free slots and returns a pointer
       to it. The object's constructor will be called.
     */
    T* allocate();

    /**
       Deallocates the given pointer that was allocated using the allocate
       method.
       @param ptr - a pointer to an object that will be deallocated
     */
    void deallocate(T* ptr);

private:
    const size_t _poolSize, _metadataSize;
    Pool _freePool;
    std::vector<Pool> _pools;

    /**
       Given a pointer to an object, returns a Pair that contains
       the pointer to the Pool in which this pointer is found and also
       at which offset it can be found in the pool of objects.
     */
    Pair getPool(void* ptr);
    /**
       Returns a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool ptr);
    /**
       Returns the number of free slots of the given Pool.
     */
    size_t getFreeCount(Pool ptr);
};

using std::vector;

template<typename T>
BitPool<T>::BitPool(size_t poolSize)
    : _poolSize(poolSize),
      _metadataSize((_poolSize >> 3) + sizeof(size_t)),
      _freePool(nullptr),
      _pools(0) {

}

template<typename T>
BitPool<T>::~BitPool() {
    for (const auto& pool : _pools) {
        std::free(pool);
    }
}

template<typename T>
T* BitPool<T>::allocate() {
    if (_freePool) {
        auto toRet = (T*) nextFree(_freePool);
        *toRet = T();
        return toRet;
    } else {
        // every pool is full, so we allocate a new pool
        Pool pool = std::malloc(_metadataSize + sizeof(T) * _poolSize);
        // we initialize the metadata of the pool
        for (size_t b = 0; b < _metadataSize; ++b) {
            char* metadata = (char*) pool + b;
            *metadata = 0;
        }
        _pools.push_back(pool);
        _freePool = pool;
        auto toRet = (T*) nextFree(pool);
        *toRet = T();
        return toRet;
    }
}

template<typename T>
void BitPool<T>::deallocate(T* ptr) {
    ptr->~T();
    auto pair = getPool(ptr);
    // pair.bits will return something like 7 for instance
    // which means that the object we deallocate will be in the
    // 7th slot of the pool pair.pool.
    // If we divide this by 8, we get the byte of metadata
    // we want to modify.
    size_t metadataOffset = pair.bits >> 3;
    char* metadata = (char*)pair.pool + metadataOffset + sizeof(size_t);
    *metadata ^= 1 << (7 - pair.bits % 8);
    --*(size_t*)pair.pool;
    // pool is empty, let's free it!
    if (getFreeCount(pair.pool) == _poolSize) {
        std::free(pair.pool);
        _pools.erase(std::find(_pools.begin(), _pools.end(), pair.pool));
    } else {
        if (!_freePool) {
            _freePool = pair.pool;
        }
    }
}

template<typename T>
Pair BitPool<T>::getPool(void* ptr) {
    for (const auto& pool : _pools) {
        char* poolByte = (char*) pool;
        if (ptr > pool &&
                ptr < poolByte + _metadataSize  + sizeof(T) * _poolSize) {
            return Pair(
                pool,
                (T*) ptr - (T*)(poolByte + _metadataSize)
            );
        }
    }
    return Pair(nullptr, 0);
}

template<typename T>
void* BitPool<T>::nextFree(Pool ptr) {
    // no use looking for an empty slot if there are none
    if (getFreeCount(ptr) != 0) {
        for (size_t b = sizeof(size_t); b < _metadataSize; ++b) {
            char* metadata = (char*) ptr + b;
            // if the b-th metadata is not 1111 1111
            // then we have a free slot
            if ((*metadata & -1) != -1) {
                for (int i = 7; i >= 0; --i) {
                    if (!((*metadata >> i) & 0x01)) {
                        *metadata |= 1 << i;
                        size_t newSize = ++*(size_t*)ptr;
                        if (newSize == _poolSize) {
                            _freePool = nullptr;
                        }
                        void* firstObj = (char*) ptr + _metadataSize;
                        return (T*) firstObj + (7 - i) +
                            ((b - sizeof(size_t)) << 3);
                    }
                }
            }
        }
    }
    return nullptr;
}

template<typename T>
size_t BitPool<T>::getFreeCount(Pool ptr) {
    return _poolSize - *(size_t*)ptr;
}

Pair::Pair(Pool poolIn, size_t bitsIn)
    : pool(poolIn), bits(bitsIn) {

}

#endif // __BIT_POOL_H__
