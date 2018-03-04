#ifndef __CUSTOM_NEW_DELETE_DEBUG_H__
#define __CUSTOM_NEW_DELETE_DEBUG_H__

#include <cstddef>

// See below the explanation of the parameters.
void* custom_new_no_throw(size_t t_size, size_t t_alignment,
                          const char* t_name, size_t t_baseSize,
                          const char* t_funcName);

/**
 * Allocate space to store an object of the given size.
 * @param t_size the size of the allocation
 * @param t_align the alignment of the allocation
 * @param t_name the name of the type allocated
 * @param t_baseSize the sizeof of the type allocated
 * @param t_funcName the name of the function which allocated the type
 */
void* custom_new(size_t t_size, size_t t_alignment,
                 const char* t_name, size_t t_baseSize,
                 const char* t_funcName);

/**
 * Frees the given pointer.
 * @param t_ptr the pointer that is freed
 */
void custom_delete(void* t_ptr) noexcept;

#endif // __CUSTOM_NEW_DELETE_DEBUG_H__
