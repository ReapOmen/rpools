/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <climits>
#include <cstddef>
#include <vector>

#ifdef __x86_64
#include "rpools/tools/light_lock.h"
#else
#include <mutex>
#include <thread>
#endif

template <typename T>
class MemoryPool
{
  public:
    /* Member types */
    typedef T               value_type;
    typedef T*              pointer;
    typedef T&              reference;
    typedef const T*        const_pointer;
    typedef const T&        const_reference;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;
    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::true_type  propagate_on_container_move_assignment;
    typedef std::true_type  propagate_on_container_swap;

    template <typename U> struct rebind {
      typedef MemoryPool<U> other;
    };

    /* Member functions */
    MemoryPool() noexcept;
    MemoryPool(const MemoryPool& memoryPool) noexcept;
    MemoryPool(MemoryPool&& memoryPool) noexcept;
    template <class U> MemoryPool(const MemoryPool<U>& memoryPool) noexcept;

    ~MemoryPool() noexcept;

    MemoryPool& operator=(const MemoryPool& memoryPool) = delete;
    MemoryPool& operator=(MemoryPool&& memoryPool) noexcept;

    pointer address(reference x) const noexcept;
    const_pointer address(const_reference x) const noexcept;

    // Can only allocate one object at a time. n and hint are ignored
    pointer allocate(size_type n = 1, const_pointer hint = 0);
    void deallocate(pointer p, size_type n = 1);

    size_type max_size() const noexcept;

    template <class U, class... Args> void construct(U* p, Args&&... args);
    template <class U> void destroy(U* p);

    template <class... Args> pointer newElement(Args&&... args);
    void deleteElement(pointer p);

  private:
    union Slot_ {
      value_type element;
      Slot_* next;
    };

#ifdef __x86_64
    light_lock_t m_lock;
#else
    std::mutex m_lock;
#endif

    typedef char* data_pointer_;
    typedef Slot_ slot_type_;
    typedef Slot_* slot_pointer_;

    slot_pointer_ currentBlock_;
    slot_pointer_ currentSlot_;
    slot_pointer_ lastSlot_;
    slot_pointer_ freeSlots_;

    size_type padPointer(data_pointer_ p, size_type align) const noexcept;
    void allocateBlock();

    static_assert(4096 >= 2 * sizeof(slot_type_), "BlockSize too small.");
};

template <typename T>
inline typename MemoryPool<T>::size_type
MemoryPool<T>::padPointer(data_pointer_ p, size_type align)
const noexcept
{
  uintptr_t result = reinterpret_cast<uintptr_t>(p);
  return ((align - result) % align);
}

template <typename T>
MemoryPool<T>::MemoryPool() noexcept
    : m_lock(
#ifdef __x86_64
          LIGHT_LOCK_INIT
#endif
      ),
      currentBlock_(nullptr),
      currentSlot_(nullptr),
      lastSlot_(nullptr),
      freeSlots_ (nullptr) {
}

template <typename T>
MemoryPool<T>::MemoryPool(const MemoryPool& memoryPool)
noexcept :
MemoryPool()
{}



template <typename T>
MemoryPool<T>::MemoryPool(MemoryPool&& memoryPool)
noexcept
    : m_lock(std::move(memoryPool.m_lock)),
      currentBlock_(std::move(memoryPool.currentBlock_)),
      currentSlot_(std::move(memoryPool.currentSlot_)),
      lastSlot_(std::move(memoryPool.lastSlot_)),
      freeSlots_(memoryPool.freeSlots_) {
  memoryPool.currentBlock_ = nullptr;
}


template <typename T>
template<class U>
MemoryPool<T>::MemoryPool(const MemoryPool<U>& memoryPool)
noexcept :
MemoryPool()
{}



template <typename T>
MemoryPool<T>&
MemoryPool<T>::operator=(MemoryPool&& memoryPool)
noexcept
{
  if (this != &memoryPool)
  {
    m_lock = memoryPool.m_lock;
    std::swap(currentBlock_, memoryPool.currentBlock_);
    currentSlot_ = memoryPool.currentSlot_;
    lastSlot_ = memoryPool.lastSlot_;
    freeSlots_ = memoryPool.freeSlots;
  }
  return *this;
}



template <typename T>
MemoryPool<T>::~MemoryPool()
noexcept
{
  slot_pointer_ curr = currentBlock_;
  while (curr != nullptr) {
    slot_pointer_ prev = curr->next;
    operator delete(reinterpret_cast<void*>(curr));
    curr = prev;
  }
}



template <typename T>
inline typename MemoryPool<T>::pointer
MemoryPool<T>::address(reference x)
const noexcept
{
  return &x;
}



template <typename T>
inline typename MemoryPool<T>::const_pointer
MemoryPool<T>::address(const_reference x)
const noexcept
{
  return &x;
}



template <typename T>
void
MemoryPool<T>::allocateBlock()
{

  // Allocate space for the new block and store a pointer to the previous one
  data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(4096));
  reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
  currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
  // Pad block body to staisfy the alignment requirements for elements
  data_pointer_ body = newBlock + sizeof(slot_pointer_);
  size_type bodyPadding = padPointer(body, alignof(slot_type_));
  currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
  lastSlot_ = reinterpret_cast<slot_pointer_>
              (newBlock + 4096 - sizeof(slot_type_) + 1);
}



template <typename T>
inline typename MemoryPool<T>::pointer
MemoryPool<T>::allocate(size_type n, const_pointer hint)
{
#ifdef __x86_64
  light_lock(&m_lock);
#else
  std::lock_guard<std::mutex> lock(m_lock);
#endif
  MemoryPool<T>::pointer toRet = nullptr;
  if (freeSlots_ != nullptr) {
    pointer result = reinterpret_cast<pointer>(freeSlots_);
    freeSlots_ = freeSlots_->next;
    toRet = result;
  }
  else {
    if (currentSlot_ >= lastSlot_)
      allocateBlock();
    toRet = reinterpret_cast<pointer>(currentSlot_++);
  }
#ifdef __x86_64
  light_unlock(&m_lock);
#endif
  return toRet;
}



template <typename T>
inline void
MemoryPool<T>::deallocate(pointer p, size_type n)
{
  if (p != nullptr) {
#ifdef __x86_64
    light_lock(&m_lock);
#else
    std::lock_guard<std::mutex> lock(m_lock);
#endif
    reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
    freeSlots_ = reinterpret_cast<slot_pointer_>(p);
#ifdef __x86_64
    light_unlock(&m_lock);
#endif
  }
}



template <typename T>
inline typename MemoryPool<T>::size_type
MemoryPool<T>::max_size()
const noexcept
{
  size_type maxBlocks = -1 / 4096;
  return (4096 - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
}



template <typename T>
template <class U, class... Args>
inline void
MemoryPool<T>::construct(U* p, Args&&... args)
{
  new (p) U (std::forward<Args>(args)...);
}



template <typename T>
template <class U>
inline void
MemoryPool<T>::destroy(U* p)
{
  p->~U();
}



template <typename T>
template <class... Args>
inline typename MemoryPool<T>::pointer
MemoryPool<T>::newElement(Args&&... args)
{
  pointer result = allocate();
  construct<value_type>(result, std::forward<Args>(args)...);
  return result;
}



template <typename T>
inline void
MemoryPool<T>::deleteElement(pointer p)
{
  if (p != nullptr) {
    p->~value_type();
    deallocate(p);
  }
}

#endif // MEMORY_POOL_H
