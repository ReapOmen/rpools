#include "AllocCollector.h"

AllocCollector::AllocCollector()
    : m_snapshotCount(0),
      m_outputFile(),
      m_allocs(),
      m_mapLock(),
      m_cv(),
      m_waitingToPrint(false),
      m_threadStarted(false) {
}

AllocCollector::~AllocCollector() {
    m_snapshotThread.detach();
    takeSnapshot();
}

void AllocCollector::addAllocation(size_t t_size) {
    if (!m_threadStarted) {
        m_threadStarted = true;
        m_snapshotThread = std::thread(&AllocCollector::run, this);
        m_outputFile.open("alloc_snapshots.output");
    }
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){return !m_waitingToPrint;});
    auto itr = m_allocs.find(t_size);
    if (itr != m_allocs.end()) {
        ++itr->second;
    } else {
        auto p = std::make_pair(t_size, 1);
        m_allocs.insert(std::move(p));
    }
    lk.unlock();
}

void AllocCollector::takeSnapshot() {
    m_waitingToPrint = true;
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_outputFile << "---- Snapshot: " << m_snapshotCount++ << "----\n";
    for (const auto& pair : m_allocs) {
        m_outputFile << "Size: " << pair.first << " Number: " << pair.second << "\n";
    }
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
