#ifndef __CUSTOM_NEW_DELETE_DEBUG_H__
#define __CUSTOM_NEW_DELETE_DEBUG_H__

#include <cstddef>

void* custom_new_no_throw(size_t size, size_t alignment, const char* name);

void* custom_new(size_t size, size_t alignment, const char* name);

void custom_delete(void* ptr) throw();

#endif // __CUSTOM_NEW_DELETE_DEBUG_H__
