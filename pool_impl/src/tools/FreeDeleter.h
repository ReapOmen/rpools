#ifndef __FREE_DELETER_H__
#define __FREE_DELETER_H__

#include <cstdlib>

template<typename T>
struct FreeDeleter {
    void operator()(T* type) {
        std::free(type);
    }
};

#endif // __FREE_DELETER_H__
