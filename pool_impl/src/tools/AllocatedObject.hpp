#ifndef __ALLOCATION_OBJECT_H__
#define __ALLOCATION_OBJECT_H__

#include <cstddef>
#include <string>
#include <map>

#include "mallocator.hpp"

struct Object {
    size_t array; // the number of contiguous objects allocated
    size_t current; // the current number of objects allocated
    size_t peak; // the peak number of objects allocated
    std::string function; // the name of the function which allocated the object
};

struct AlignedObject {
    size_t baseSize; // the sizeof of our given object
    // a map from allocation sizes to actual Objects
    std::map<size_t, Object, std::less<size_t>,
             mallocator<std::pair<const size_t, Object>>> sizes;
};

/**
 * Holds all the metadata of an allocation.
 */
struct AllocatedObject {
    // a map from alignments to AlignedObjects
    std::map<size_t, AlignedObject, std::less<size_t>,
             mallocator<std::pair<const size_t, AlignedObject>>> alignments;
};

#endif // __ALLOCATION_OBJECT_H__
