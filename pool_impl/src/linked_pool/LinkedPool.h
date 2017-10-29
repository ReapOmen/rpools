#ifndef __LINKED_POOL_H__
#define __LINKED_POOL_H__

#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <unordered_set>

using Pool = void*;

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
       Creates a LinkedPool allocator that will allocate objects of type T
       in pools and return pointers to them.
     */
    LinkedPool();

    ~LinkedPool() = default;

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
    // the extra data that we keep for each pool
    static constexpr size_t METADATA_SIZE = sizeof(void*) + sizeof(size_t);
    // because we allocate memory in chunks of 4096 bytes,
    // we are only interested in the first 52 high order bits
    static constexpr size_t POOL_MASK = 18446744073709547520U; // -1 >> 12 << 12
    // XXX use constant defined in some linux header instead of
    // hardcoding the value
    static constexpr size_t PAGE_SIZE = 4096;

    const size_t _poolSize;
    Pool _freePool;
    std::unordered_set<Pool> _freePools;

    /**
       Returns a pointer to the next free slot of memory from the given Pool.
     */
    void* nextFree(Pool ptr);
};

template<typename T>
LinkedPool<T>::LinkedPool()
    : _poolSize((PAGE_SIZE - METADATA_SIZE) / sizeof(T)),
      _freePool(nullptr),
      _freePools() {
}

template<typename T>
T* LinkedPool<T>::allocate() {
    if (_freePool) {
        return new(nextFree(_freePool)) T();
    } else if (_freePools.size() > 0) {
        _freePool = *_freePools.begin();
         return new(nextFree(_freePool)) T();
    } else {
        // create a new pool because there are no free pool slots left
        Pool pool = mmap(NULL, PAGE_SIZE,
            PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // init all the metadata
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
        _freePools.insert(pool);
        _freePool = pool;
        return new(nextFree(pool)) T();
    }
}

template<typename T>
void LinkedPool<T>::deallocate(T* ptr) {
    // destroy T and create a Node instead
    ptr->~T();
    Node* newNode = reinterpret_cast<Node*>(ptr);
    *newNode = Node();
    // get the pool of ptr
    Pool pool = (Pool) ((size_t) ptr & POOL_MASK);
    // update nodes to point to the newly create Node
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
    // pool is empty, let's free it!
    size_t newSize = --*(size_t*)pool;
    if (newSize == 0) {
        _freePools.erase(pool);
        munmap(pool, PAGE_SIZE);
    } else {
        _freePool = pool;
        if (newSize == _poolSize - 1) {
            _freePools.insert(_freePool);
        }
    }
}

template<typename T>
void* LinkedPool<T>::nextFree(Pool pool) {
    Node* head = ((Node*)((char*)pool + sizeof(size_t)));
    if (head->next) {
        void* toReturn = head->next;
        head->next = head->next->next;
        if (++*(size_t*)(pool) == _poolSize) {
            _freePools.erase(pool);
            _freePool = _freePools.size() == 0 ? nullptr : *_freePools.begin();
        }
        return toReturn;
    }
    return head->next;
}

#endif // __LINKED_POOL_H__
