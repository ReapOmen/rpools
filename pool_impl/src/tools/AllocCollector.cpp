#include "AllocCollector.h"
#include "proc_utils.h"

AllocCollector::AllocCollector()
    : m_snapshotCount(0),
      m_overheads(0),
      m_outputFile(),
      m_objectsFile(),
      m_allocs(),
      m_objects(),
      m_mapLock(),
      m_cv(),
      m_waitingToPrint(false),
      m_threadStarted(false) {
}

AllocCollector::~AllocCollector() {
    m_snapshotThread.detach();
    takeSnapshot();
}

void AllocCollector::addObject(size_t t_size) {
    if (!m_threadStarted) {
        m_threadStarted = true;
        m_snapshotThread = std::thread(&AllocCollector::run, this);
        m_outputFile.open("memory_snapshots_" + std::to_string(getpid()) + ".output");
        // XXX hardcoded description
        m_outputFile << "desc: (none)\ncmd: /home/robert/rm_me/assignment5-ReapOmen/BestFSSudoku\ntime_unit: i\n";
        m_objectsFile.open("object_snapshots_" + std::to_string(getpid()) + ".output");
    }
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){return !m_waitingToPrint;});
    auto itr = m_objects.find(t_size);
    if (itr != m_objects.end()) {
        ++itr->second;
    } else {
        m_objects.insert(std::make_pair(t_size, 1));
    }
    lk.unlock();
}

void AllocCollector::removeObject(size_t t_size) {
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){return !m_waitingToPrint;});
    auto itr = m_objects.find(t_size);
    --itr->second;
    lk.unlock();
}

void AllocCollector::addAllocation(size_t t_size) {
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){return !m_waitingToPrint;});
    auto itr = m_allocs.find(t_size);
    if (itr != m_allocs.end()) {
        ++itr->second;
    } else {
        m_allocs.insert(std::make_pair(t_size, 1));
    }
    lk.unlock();
}

void AllocCollector::removeAllocation(size_t t_size) {
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){return !m_waitingToPrint;});
    auto itr = m_allocs.find(t_size);
    --itr->second;
    lk.unlock();
}

void AllocCollector::addOverhead(size_t t_size) {
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){return !m_waitingToPrint;});
    m_overheads += t_size;
    lk.unlock();
}

void AllocCollector::removeOverhead(size_t t_size) {
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){return !m_waitingToPrint;});
    m_overheads -= t_size;
    lk.unlock();
}

void AllocCollector::takeSnapshot() {
    m_waitingToPrint = true;
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_outputFile << "#-----------\n";
    m_outputFile << "snapshot=" << m_snapshotCount << '\n';
    m_outputFile << "#-----------\n";
    m_outputFile << "time=" << m_snapshotCount * 10 << '\n';
    m_objectsFile << "#-----------\n";
    m_objectsFile << "snapshot=" << m_snapshotCount << '\n';
    m_objectsFile << "#-----------\n";
    size_t totalMem = 0;
    for (const auto& pair : m_allocs) {
        totalMem += pair.first * pair.second;
    }
    for (const auto& pair : m_objects) {
        m_objectsFile << "Objects of size " << pair.first
                      << ": " << pair.second << '\n';
    }
    m_outputFile << "mem_heap_B=" << totalMem << '\n';
    m_outputFile << "mem_heap_extra_B=" << m_overheads << '\n';
    m_outputFile << "mem_stacks_B=" << 0 << '\n';
    m_outputFile << "heap_tree=empty\n";
    ++m_snapshotCount;
    m_waitingToPrint = false;
    lk.unlock();
    m_cv.notify_one();
}

void AllocCollector::run() {
    while (true) {
        takeSnapshot();
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }
}
