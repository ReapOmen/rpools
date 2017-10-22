#include "AllocFile.h"
#include "SomeObject.h"
#include "SomeObject2.h"
#include "SomeObject3.h"
#include "Obj1.h"
#include "Obj2.h"

#include <iostream>
using std::cout;
using std::endl;

AllocFile::AllocFile()
    : allocs(),
      deallocs(),
      allocFile("allocation_file.txt"),
      deallocFile("deallocation_file.txt"),
      overheadFile("overheads.txt") {

}

AllocFile::~AllocFile() {
    for (const auto& entry : allocs) {
        allocFile << entry.second  << " allocations of size "<<
            entry.first << " have been made.\n";
    }
    for (const auto& entry : deallocs) {
        deallocFile << entry.second  << " deallocations of size "<<
            entry.first << " have been made.\n";
    }
    overheadFile << "SomeObject1 maximum overhead: " <<SomeObject::overhead << endl;
    overheadFile << "SomeObject2 maximum overhead: " <<SomeObject2::overhead << endl;
    overheadFile << "SomeObject3 maximum overhead: " <<SomeObject3::overhead << endl;
    overheadFile << "Obj1 maximum overhead: " <<Obj1::overhead << endl;
#ifdef WRITE_ALLOCS_TO_FILE
    overheadFile << "Obj2 maximum overhead: " <<Obj2::overhead << endl;
#endif
}

void AllocFile::processAllocation(size_t size) {
    if (allocs.find(size) != allocs.end()) {
        allocs[size] += 1;
    } else {
        allocs.insert(std::make_pair(size, 1));
    }
}

void AllocFile::processDeallocation(size_t size) {
    if (size != 0) {
        if (deallocs.find(size) != deallocs.end()) {
            deallocs[size] += 1;
        } else {
            deallocs.insert(std::make_pair(size, 1));
        }
    }
}

AllocFile allocFile;
