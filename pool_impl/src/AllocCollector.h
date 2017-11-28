#ifndef __ALLOC_COLLECTOR_H__
#define __ALLOC_COLLECTOR_H__

#include <map>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include "CustomAlloc.h"

class AllocCollector {
public:
    AllocCollector();
    void addAllocation(size_t t_size);
    void removeAllocation(size_t t_size);
    void addOverhead(size_t t_size);
    void removeOverhead(size_t t_size);
    void takeSnapshot();
    virtual ~AllocCollector();
private:
    size_t m_snapshotCount;
    size_t m_overheads;
    std::ofstream m_outputFile;
    std::map<size_t, size_t,
             std::less<size_t>,
             mallocator<std::pair<const size_t, size_t>>> m_allocs;
    std::mutex m_mapLock;
    std::condition_variable m_cv;
    bool m_waitingToPrint, m_threadStarted;
    std::thread m_snapshotThread;

    void run();
};

#endif // __ALLOC_COLLECTOR_H__
