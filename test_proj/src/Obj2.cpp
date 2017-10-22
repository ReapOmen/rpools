#include "Obj2.h"
#include <algorithm>
using std::make_pair;
using std::pair;
using std::vector;
using std::max;

constexpr size_t SIZE_OF_UNSGN = sizeof(size_t);
constexpr size_t SIZE_OF_VOID = sizeof(void*);

size_t Obj2::POOL_SIZE = 80;
size_t Obj2::METADATA_SIZE = (Obj2::POOL_SIZE >> 3) + SIZE_OF_UNSGN;
vector<void*> Obj2::pools;

#ifdef WRITE_ALLOCS_TO_FILE
size_t Obj2::overhead = 0;

size_t Obj2::calculateOverhead() {
    return 16 + METADATA_SIZE +
        sizeof(vector<void*>) + pools.size() * SIZE_OF_VOID;
}
#endif

pair<void*, size_t> Obj2::getPool(void* ptr) {
    for (const auto& pool : pools) {
        char* poolByte = (char*) pool;
        if (ptr > pool &&
                ptr < poolByte + METADATA_SIZE  + sizeof(Obj2) * POOL_SIZE) {
            return make_pair(
                pool,
                (Obj2*) ptr - (Obj2*)(poolByte + METADATA_SIZE)
            );
        }
    }
    return make_pair(nullptr, 0);
}

size_t Obj2::getFreeCount(void* ptr) {
    return POOL_SIZE - *(size_t*)ptr;
}

void* Obj2::nextFree(void* ptr) {
    for (size_t b = sizeof(size_t); b < METADATA_SIZE; ++b) {
        char* metadata = (char*) ptr + b;
        if ((*metadata & -1) != -1) {
            for (int i = 7; i >= 0; --i) {
                if (!((*metadata >> i) & 0x01)) {
                    *metadata |= 1 << i;
                    ++*(size_t*)ptr;
                    void* firstObj = (char*) ptr + METADATA_SIZE;
                    return (Obj2*)firstObj + (7 - i) +
                        ((b - sizeof(size_t)) << 3);
                }
            }
        }
    }
    return nullptr;
}

void* Obj2::operator new(size_t size) {
    for (const auto& pool : pools) {
        void* free = nextFree(pool);
        if (free != nullptr) {
            return free;
        }
    }
    void* pool = std::malloc(METADATA_SIZE + size * POOL_SIZE);
    for (size_t b = 0; b < METADATA_SIZE; ++b) {
        char* metadata = (char*) pool + b;
        *metadata = 0;
    }
    pools.push_back(pool);
#ifdef WRITE_ALLOCS_TO_FILE
    ::allocFile.processAllocation(METADATA_SIZE + size * POOL_SIZE);
    overhead = std::max(calculateOverhead(), overhead);
#endif
    return nextFree(pool);
}

void Obj2::operator delete(void* ptr) {
    reinterpret_cast<Obj2*>(ptr)->~Obj2();
    auto pair = getPool(ptr);
    size_t metadataOffset = pair.second >> 3;
    char* metadata = (char*)pair.first + metadataOffset + SIZE_OF_UNSGN;
    *metadata ^= 1 << (7 - pair.second % 8);
    --*(size_t*)pair.first;
    if (getFreeCount(pair.first) == POOL_SIZE) {
        std::free(pair.first);
#ifdef WRITE_ALLOCS_TO_FILE
        ::allocFile.processDeallocation(METADATA_SIZE +
                                        sizeof(Obj2) * POOL_SIZE);
#endif
        pools.erase(std::find(pools.begin(), pools.end(), pair.first));
    }
#ifdef WRITE_ALLOCS_TO_FILE
    overhead = std::max(calculateOverhead(), overhead);
#endif
}
