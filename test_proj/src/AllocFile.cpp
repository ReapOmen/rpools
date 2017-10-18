#include "AllocFile.h"

AllocFile::AllocFile()
    : allocs(),
      deallocs(),
      allocFile("allocation_file.txt"),
      deallocFile("deallocation_file.txt") {

}

AllocFile::~AllocFile() {
    for (const auto& entry : allocs) {
        allocFile << entry.second  << " allocations of size "<< entry.first << " have been made.\n";
    }
    for (const auto& entry : deallocs) {
        deallocFile << entry.second  << " deallocations of size "<< entry.first << " have been made.\n";
    }
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
