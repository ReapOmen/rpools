#include "rpools/tools/LMLock.hpp"
#include <utility>

using namespace rpools;

LMLock::LMLock()
    : m_lock(
#ifdef __x86_64
             LIGHT_LOCK_INIT
#endif
      ) { }

LMLock::LMLock(LMLock&& other)
    : m_lock(std::move(other.m_lock)) { }

LMLock& LMLock::operator =(LMLock&& other) {
    m_lock = std::move(other.m_lock);
    return *this;
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
