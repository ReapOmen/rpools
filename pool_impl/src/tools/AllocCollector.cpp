#include "AllocCollector.h"
#include "proc_utils.h"

AllocCollector::AllocCollector()
    : m_snapshotCount(0),
      m_objectsFile(),
      m_name(),
      m_numOfObjects(),
      m_snapshots(),
      m_mapLock(),
      m_cv(),
      m_waitingToPrint(false),
      m_threadStarted(false) {
}

AllocCollector::~AllocCollector() {
    if (m_threadStarted) {
        takeSnapshot();
        m_objectsFile << m_snapshots.dump(4);
        m_snapshotThread.detach();
    }
}

void AllocCollector::addObject(size_t t_size, size_t t_align,
                               const char* t_name, void* t_ptr) {
    if (!m_threadStarted) {
        m_threadStarted = true;
        m_snapshotThread = std::thread(&AllocCollector::run, this);
        m_objectsFile.open("object_snapshots_" + std::to_string(getpid()) + ".output");
    }
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){ return !m_waitingToPrint; });
    std::string name(t_name);
    name += " (size:" + std::to_string(t_size) + ", alignment:"
        + std::to_string(t_align)  + ")";
    m_name[t_ptr] = name;
    auto& objStat = m_numOfObjects[name];
    ++objStat.first;
    objStat.second = std::max(objStat.first, objStat.second);
    lk.unlock();
}

void AllocCollector::removeObject(void* t_ptr) {
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){ return !m_waitingToPrint; });
    auto& objStat = m_numOfObjects[m_name[t_ptr]];
    --objStat.first;
    m_name.erase(t_ptr);
    lk.unlock();
}

void AllocCollector::takeSnapshot() {
    m_waitingToPrint = true;
    std::unique_lock<std::mutex> lk(m_mapLock);
    for (const auto& pair : m_numOfObjects) {
        auto& entry = m_snapshots[m_snapshotCount][pair.first];
        entry["current"] = pair.second.first;
        entry["peak"] = pair.second.second;
    }
    ++m_snapshotCount;
    m_waitingToPrint = false;
    lk.unlock();
    m_cv.notify_one();
}

void AllocCollector::run() {
    while (true) {
        takeSnapshot();
        std::chrono::milliseconds timespan(100);
        std::this_thread::sleep_for(timespan);
    }
}
