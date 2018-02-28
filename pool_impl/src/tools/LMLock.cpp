#include "LMLock.hpp"

LMLock::LMLock()
    : m_lock(
#ifdef __x86_64
             LIGHT_LOCK_INIT
#endif
      ) {
}

void LMLock::lock() {
#ifdef __x86_64
    light_lock(&m_lock);
#else
    m_lock.lock();
#endif
}

void LMLock::unlock() {
#ifdef __x86_64
    light_unlock(&m_lock);
#else
    m_lock.unlock();
#endif
}
