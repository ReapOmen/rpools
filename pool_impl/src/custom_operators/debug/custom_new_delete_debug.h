#ifndef __CUSTOM_NEW_DELETE_DEBUG_H__
#define __CUSTOM_NEW_DELETE_DEBUG_H__

#include <cstddef>

void* custom_new_no_throw(size_t t_size, size_t t_alignment,
                          const char* t_name, size_t t_baseSize,
                          const char* t_funcName);

void* custom_new(size_t t_size, size_t t_alignment,
                 const char* t_name, size_t t_baseSize,
                 const char* t_funcName);

void custom_delete(void* t_ptr) throw();

#endif // __CUSTOM_NEW_DELETE_DEBUG_H__
