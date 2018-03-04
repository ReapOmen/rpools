#ifndef __L_M_LOCK_H__
#define __L_M_LOCK_H__

#ifdef __x86_64
#include "light_lock.h"
#else
#include <mutex>
#include <thread>
#endif

/**
 *  Represents a locking mechanism which uses light_lock_t on x86 systems
 *  and std::mutex on other systems.
 */
class LMLock {
public:
    LMLock();
    LMLock(const LMLock& other) = delete;
    void lock();
    void unlock();
    virtual ~LMLock() = default;
private:
#ifdef __x86_64
    light_lock_t m_lock;
#else
    std::mutex m_lock;
#endif
};

#endif // __L_M_LOCK_H__
