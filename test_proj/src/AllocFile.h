#ifndef __ALLOC_FILE_H__
#define __ALLOC_FILE_H__

#include <fstream>
#include <map>

class AllocFile {
public:

    AllocFile();

    virtual ~AllocFile();

    void processAllocation(size_t size);

    void processDeallocation(size_t size = 0);

private:
    std::map<size_t, size_t> allocs, deallocs;
    std::ofstream allocFile, deallocFile, overheadFile;
};

extern AllocFile allocFile;

#endif // __ALLOC_FILE_H__
