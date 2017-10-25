#include "Obj3.h"
#include <algorithm>
using std::make_pair;
using std::pair;
using std::vector;
using std::max;

constexpr size_t SIZE_OF_UNSGN = sizeof(size_t);
constexpr size_t SIZE_OF_VOID = sizeof(void*);

size_t Obj3::POOL_SIZE = 80;
size_t Obj3::METADATA_SIZE = SIZE_OF_VOID + SIZE_OF_UNSGN;
vector<void*> Obj3::pools;

#ifdef WRITE_ALLOCS_TO_FILE
size_t Obj3::overhead = 0;

size_t Obj3::calculateOverhead() {
    return 16 + METADATA_SIZE +
        sizeof(vector<void*>) + pools.size() * SIZE_OF_VOID;
}
#endif

void* Obj3::getPool(void* ptr) {
    for (const auto& pool : pools) {
        char* poolByte = (char*) pool;
        if (ptr > pool &&
                ptr < poolByte + METADATA_SIZE  + sizeof(Obj3) * POOL_SIZE) {
            return pool;

        }
    }
    return nullptr;
}

size_t Obj3::getFreeCount(void* ptr) {
    return POOL_SIZE - *(size_t*)ptr;
}

void* Obj3::nextFree(void* ptr) {
    Node* head = ((Node*)((char*)ptr + SIZE_OF_UNSGN));
    if (head->next != nullptr) {
        void* toReturn = head->next;
        head->next = head->next->next;
        ++*(size_t*)(ptr);
        return toReturn;
    }
    return head->next;
}

void* Obj3::operator new(size_t size) {
    for (const auto& pool : pools) {
        void* free = nextFree(pool);
        if (free != nullptr) {
            return free;
        }
    }
    void* pool = std::malloc(METADATA_SIZE + size * POOL_SIZE);
    *(size_t*)pool = 0;
    Node* head = (Node*)((char*)pool + SIZE_OF_UNSGN);
    *head = Node();
    head->next = head + 1;
    Obj3* first = reinterpret_cast<Obj3*>(head->next);
    for (int i = 0; i < POOL_SIZE - 1; ++i) {
        Node* node = reinterpret_cast<Node*>(first);
        *node = Node();
        node->next = reinterpret_cast<Node*>(++first);
    }
    Node* node = reinterpret_cast<Node*>(first);
    *node = Node();
    pools.push_back(pool);
#ifdef WRITE_ALLOCS_TO_FILE
    ::allocFile.processAllocation(METADATA_SIZE + size * POOL_SIZE);
    overhead = std::max(calculateOverhead(), overhead);
#endif
    return nextFree(pool);
}

void Obj3::operator delete(void* ptr) {
    reinterpret_cast<Obj3*>(ptr)->~Obj3();
    Node* newNode = reinterpret_cast<Node*>(ptr);
    void* pool = getPool(ptr);
    *newNode = Node();
    Node* head = reinterpret_cast<Node*>((char*)pool + SIZE_OF_UNSGN);
    if (head->next == nullptr) {
        head->next = newNode;
    } else {
        while (ptr < head) {
            head = head->next;
        }
        newNode->next = head->next;
        head->next = newNode;
    }
    --*(size_t*)pool;
    if (getFreeCount(pool) == POOL_SIZE) {
        std::free(pool);
#ifdef WRITE_ALLOCS_TO_FILE
        ::allocFile.processDeallocation(METADATA_SIZE +
                                        sizeof(Obj3) * POOL_SIZE);
#endif
        pools.erase(std::find(pools.begin(), pools.end(), pool));
    }
#ifdef WRITE_ALLOCS_TO_FILE
    overhead = std::max(calculateOverhead(), overhead);
#endif
}
