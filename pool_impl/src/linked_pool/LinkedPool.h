#ifndef __LINKED_POOL_H__
#define __LINKED_POOL_H__

#include <vector>
#include <cstdlib>
#include <algorithm>

using Pool = void*;

/**
   Used by getPool.
 */
struct Node {
    Node* next;
};

/**
   LinkedPool is a pool allocation system which tries to minimise the amount
   of overheads created by allocating lots of objects on the heap.
 */
template<typename T>
class LinkedPool {
public:

    /**
       Creates a LinkedPool allocator that will create pools of the given size.
       By default, poolSize is 80.
       @param poolSize - the size of the pools that will be
                         allocated (80 by default)
     */
    LinkedPool(size_t poolSize = 80);

    ~LinkedPool();

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
       Given a pointer to an object,
       returns the Pool in which it is contained.
     */
    Pool getPool(void* ptr);
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
LinkedPool<T>::LinkedPool(size_t poolSize)
    : _poolSize(poolSize),
      _metadataSize(sizeof(void*) + sizeof(size_t)),
      _freePool(nullptr),
      _pools(0) {

}

template<typename T>
LinkedPool<T>::~LinkedPool() {
    for (const auto& pool : _pools) {
        std::free(pool);
    }
}

template<typename T>
T* LinkedPool<T>::allocate() {
    if (_freePool) {
        auto toRet = (T*) nextFree(_freePool);
        *toRet = T();
        return toRet;
    } else {
        // every pool is full, so we allocate a new pool
        Pool pool = std::malloc(_metadataSize + sizeof(T) * _poolSize);
        *(size_t*)pool = 0;
        Node* head = (Node*)((char*)pool + sizeof(size_t));
        *head = Node();
        head->next = head + 1;
        T* first = reinterpret_cast<T*>(head->next);
        for (int i = 0; i < _poolSize - 1; ++i) {
            Node* node = reinterpret_cast<Node*>(first);
            *node = Node();
            node->next = reinterpret_cast<Node*>(++first);
        }
        Node* node = reinterpret_cast<Node*>(first);
        *node = Node();
        _pools.push_back(pool);
        _freePool = pool;
        auto toRet = (T*) nextFree(pool);
        *toRet = T();
        return toRet;
    }
}

template<typename T>
void LinkedPool<T>::deallocate(T* ptr) {
    ptr->~T();
    Node* newNode = reinterpret_cast<Node*>(ptr);
    Pool pool = getPool(ptr);
    *newNode = Node();
    Node* head = reinterpret_cast<Node*>((char*)pool + sizeof(size_t));
    if (head->next == nullptr) {
        head->next = newNode;
    } else {
        void* vPtr = (void*) ptr;
        while (vPtr < head) {
            head = head->next;
        }
        newNode->next = head->next;
        head->next = newNode;
    }
    --*(size_t*)pool;
    // pool is empty, let's free it!
    if (getFreeCount(pool) == _poolSize) {
        std::free(pool);
        _pools.erase(std::find(_pools.begin(), _pools.end(), pool));
    } else {
        if (!_freePool) {
            _freePool = pool;
        }
    }
}

template<typename T>
Pool LinkedPool<T>::getPool(void* ptr) {
    for (const auto& pool : _pools) {
        char* poolByte = (char*) pool;
        if (ptr > pool &&
                ptr < poolByte + _metadataSize  + sizeof(T) * _poolSize) {
            return pool;
        }
    }
    return nullptr;
}

template<typename T>
void* LinkedPool<T>::nextFree(Pool ptr) {
    Node* head = ((Node*)((char*)ptr + sizeof(size_t)));
    if (head->next != nullptr) {
        void* toReturn = head->next;
        head->next = head->next->next;
        size_t newSize = ++*(size_t*)(ptr);
        if (newSize == _poolSize) {
            _freePool = nullptr;
        }
        return toReturn;
    }
    return head->next;
}

template<typename T>
size_t LinkedPool<T>::getFreeCount(Pool ptr) {
    return _poolSize - *(size_t*)ptr;
}

#endif // __LINKED_POOL_H__
