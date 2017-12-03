#ifndef __CUSTOM_ALLOC_H__
#define __CUSTOM_ALLOC_H__

#include <cstdlib>
#include <memory>
#include <limits>

// from
// https://stackoverflow.com/questions/21081796/why-not-to-inherit-from-stdallocator
template <typename T>
struct mallocator {
    using value_type = T;

    mallocator() = default;
    template <class U>
    mallocator(const mallocator<U>&) {}

    T* allocate(std::size_t n) {
        if (n <= std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            if (auto ptr = std::malloc(n * sizeof(T))) {
                return static_cast<T*>(ptr);
            }
        }
        throw std::bad_alloc();
    }

    void deallocate(T* ptr, std::size_t n) {
        std::free(ptr);
    }

    template <typename U>
    using  rebind = mallocator<U>;
};

template <typename T, typename U>
inline bool operator == (const mallocator<T>&, const mallocator<U>&) {
    return true;
}

template <typename T, typename U>
inline bool operator != (const mallocator<T>& a, const mallocator<U>& b) {
    return !(a == b);
}

#endif // __CUSTOM_ALLOC_H__
