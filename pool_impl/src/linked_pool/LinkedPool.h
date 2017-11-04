#ifndef __LINKED_POOL_H__
#define __LINKED_POOL_H__

#include <cstdlib>
#include <unistd.h>
#include <unordered_set>

using Pool = void*;

struct Node {
    Node* next;
};

struct PoolHeader {
    size_t sizeOfPool;
    Node* head;

    PoolHeader(char* start)
        : sizeOfPool(0),
          head(new(start + sizeof(size_t)) Node()) {
    }
};

/**
   LinkedPool is a pool allocation system which tries to minimise the amount
   of overheads created by allocating lots of objects on the heap.
 */
template<typename T>
class LinkedPool {
public:

    /**
       Creates a LinkedPool allocator that will allocate objects of type T
       in pools and return pointers to them.
     */
    LinkedPool();

    /**
       Allocates an object in one of the free slots and returns a pointer
       to it. The object's default constructor will be called.
       @return A pointer to the newly created object of type T.
     */
    void* allocate();

    /**
       Deallocates the given pointer that was allocated using the allocate
       method.
       @param ptr - a pointer to an object that will be deallocated
     */
    void deallocate(void* ptr);

private:
    // because we allocate memory in chunks of 4096 bytes,
    // we are only interested in the first 52 high order bits
    static constexpr size_t POOL_MASK = 18446744073709547520U; // -1 >> 12 << 12
    static const size_t PAGE_SIZE;

    const size_t _poolSize;
    Pool _freePool;
    std::unordered_set<Pool> _freePools;

    /**
       Returns a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool ptr);
};

template<typename T>
const size_t LinkedPool<T>::PAGE_SIZE = sysconf(_SC_PAGESIZE);

template<typename T>
LinkedPool<T>::LinkedPool()
    : _poolSize((PAGE_SIZE - sizeof(PoolHeader)) / sizeof(T)),
      _freePool(nullptr),
      _freePools() {
}

template<typename T>
void* LinkedPool<T>::allocate() {
    if (_freePool) {
        return nextFree(_freePool);
    } else if (_freePools.size() > 0) {
        _freePool = *_freePools.begin();
         return nextFree(_freePool);
    } else {
        // create a new pool because there are no free pool slots left
        Pool pool = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        // init all the metadata
        PoolHeader* header = (PoolHeader*) pool;
        *header = PoolHeader((char*) pool);
        Node* head = header->head;
        head->next = head + 1;
        T* first = reinterpret_cast<T*>(head->next);
        for (int i = 0; i < _poolSize - 1; ++i) {
            Node* node = reinterpret_cast<Node*>(first);
            *node = Node();
            node->next = reinterpret_cast<Node*>(++first);
        }
        Node* node = reinterpret_cast<Node*>(first);
        *node = Node();
        _freePools.insert(pool);
        _freePool = pool;
        return nextFree(pool);
    }
}

template<typename T>
void LinkedPool<T>::deallocate(void* ptr) {
    Node* newNode = reinterpret_cast<Node*>(ptr);
    *newNode = Node();
    // get the pool of ptr
    PoolHeader* pool = (PoolHeader*) ((size_t) ptr & POOL_MASK);
    // update nodes to point to the newly create Node
    Node* head = (Node*) &pool->head;
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
    // pool is empty, let's free it!
    size_t newSize = --pool->sizeOfPool;
    if (newSize == 0) {
        _freePools.erase(pool);
        free(pool);
        _freePool = _freePools.size() == 0 ? nullptr : *_freePools.begin();
    } else {
        _freePool = pool;
        if (newSize == _poolSize - 1) {
            _freePools.insert(_freePool);
        }
    }
}

template<typename T>
void* LinkedPool<T>::nextFree(Pool pool) {
    PoolHeader* header = (PoolHeader*) pool;
    Node* head = (Node*) &header->head;
    if (head->next) {
        void* toReturn = head->next;
        head->next = head->next->next;
        if (++(header->sizeOfPool) == _poolSize) {
            _freePools.erase(pool);
            _freePool = _freePools.size() == 0 ? nullptr : *_freePools.begin();
        }
        return toReturn;
    }
    return head->next;
}

#endif // __LINKED_POOL_H__
