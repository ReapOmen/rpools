#include "AllocCollector.hpp"
#include "proc_utils.hpp" // getpid

AllocCollector::AllocCollector()
    : m_objectsFile(),
      m_allocObj(),
      m_obj(),
      m_snapshots(),
      m_mapLock(),
      m_cv() {
}

AllocCollector::~AllocCollector() {
    if (m_threadStarted) {
        takeSnapshot();
        m_objectsFile << m_snapshots.dump(4);
        m_snapshotThread.detach();
    }
}

void AllocCollector::addObject(size_t t_size, size_t t_align,
                               const char* t_name, size_t t_baseSize,
                               const char* t_funcName, void* t_ptr) {
    if (!m_threadStarted) {
        m_threadStarted = true;
        m_snapshotThread = std::thread(&AllocCollector::run, this);
        m_objectsFile.open("object_snapshots_" + std::to_string(getpid()) +
                           ".json");
    }
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){ return !m_waitingToPrint; });
    std::string name(t_name);
    AlignedObject& alignedObj = m_allocObj[name].alignments[t_align];
    alignedObj.baseSize = t_baseSize;
    Object& obj = alignedObj.sizes[t_size];
    ++obj.current;
    obj.peak = std::max(obj.peak, obj.current);
    obj.function = t_funcName;
    if (t_baseSize != 0) {
        obj.array = t_size / t_baseSize;
    }
    m_obj[t_ptr] = &obj;
    lk.unlock();
}

void AllocCollector::removeObject(void* t_ptr) {
    std::unique_lock<std::mutex> lk(m_mapLock);
    m_cv.wait(lk, [&](){ return !m_waitingToPrint; });
    Object* objStat = m_obj[t_ptr];
    --objStat->current;
    m_obj.erase(t_ptr);
    lk.unlock();
}

void AllocCollector::takeSnapshot() {
    m_waitingToPrint = true;
    std::unique_lock<std::mutex> lk(m_mapLock);
    // allocObj -> pair<name, AllocatedObject>
    for (const auto& allocObj : m_allocObj) {
        // alignedObj -> pair<alignment, AlignedObject>
        for (const auto& alignedObj : allocObj.second.alignments) {
            // obj -> pair<size, Object>
            for (const auto& obj : alignedObj.second.sizes) {
                auto& entry = m_snapshots[m_snapshotCount][allocObj.first]
                    [std::to_string(alignedObj.first)]
                    [std::to_string(obj.first)];
                entry["base_size"] = alignedObj.second.baseSize;
                entry["array"] = obj.second.array;
                entry["current"] = obj.second.current;
                entry["peak"] = obj.second.peak;
                entry["function"] = obj.second.function;
            }
        }
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
