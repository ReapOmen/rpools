/**
 *  @file custom_new_delete.hpp
 *  A replacement for `operator new/delete`.
 */

#ifndef __CUSTOM_NEW_DELETE_H__
#define __CUSTOM_NEW_DELETE_H__

#include <new>
#include <cstddef>

/**
 *  Allocates `t_size` bytes and aligns it according to `t_alignment`.
 *  @note This function will return a nullptr when allocation fails.
 *  @param t_size the size of the allocation
 *  @param t_alignment the alignment of the allocation which cannot
 *                     be greater than 16
 *  @return a pointer aligned to `t_alignment` of size `t_size`.
 */
void* custom_new_no_throw(size_t t_size,
                          size_t t_alignment=alignof(max_align_t));

/**
 *  Allocates `t_size` bytes and aligns it according to `t_alignment`.
 *  @note This function throws bad_alloc when allocation fails.
 *  @param t_size the size of the allocation
 *  @param t_alignment the alignment of the allocation which cannot
 *                     be greater than 16
 *  @return a pointer aligned to `t_alignment` of size `t_size`.
 */
void* custom_new(size_t t_size,
                 size_t t_alignment=alignof(max_align_t));

/**
 *  Frees up the memory that starts at `t_ptr`.
 *  @param t_ptr the pointer that is freed
 */
void custom_delete(void* t_ptr) noexcept;

#endif // __CUSTOM_NEW_DELETE_H__
