#ifndef __ALLOC_COLLECTOR_H__
#define __ALLOC_COLLECTOR_H__

#include <map>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include "nlohmann/json.hpp"
#include "mallocator.h"

class AllocCollector {
public:
    AllocCollector();
    void addObject(size_t t_size, size_t t_align,
                   const char* t_name, void* t_ptr);
    void removeObject(void* t_ptr);
    void takeSnapshot();
    virtual ~AllocCollector();
private:
    size_t m_snapshotCount;
    std::ofstream m_objectsFile;
    std::map<void*, std::string, std::less<void*>,
             mallocator<std::pair<const void*, std::string>>> m_name;
    std::map<std::string, std::pair<size_t, size_t>,
             std::less<std::string>,
             mallocator<std::pair<std::string, std::pair<size_t, size_t>>>
             > m_numOfObjects;
    nlohmann::json m_snapshots;
    std::mutex m_mapLock;
    std::condition_variable m_cv;
    std::thread m_snapshotThread;
    bool m_waitingToPrint, m_threadStarted;

    void run();
};

#endif // __ALLOC_COLLECTOR_H__
