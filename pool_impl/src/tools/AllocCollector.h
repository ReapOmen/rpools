#ifndef __ALLOC_COLLECTOR_H__
#define __ALLOC_COLLECTOR_H__

#include <map>
#include <fstream> // ofstream
#include <mutex>
#include <condition_variable>
#include <thread>

#include "nlohmann/json.hpp" // basic_json
#include "AllocatedObject.h"

/**
 * Collects information about all the allocations that are made with
 * custom_new.
 * The allocation's metadata is stored and every 100ms a snapshot of all
 * the allocations made is saved.
 * @par
 * When the object dies, the snapshots are written to a file called
 * `object_snapshots_<PID>`.
 * @par
 * An example JSON which represents a snapshot:
 * ```
 * ...
 * "Sudoku": { // the name of the type allocated
 *     "8": { // the alignment
 *         "48": { // the size of the allocation (`array * sizeof(Sudoku)`)
 *             "array": 1, // the number of contiguous elements allocated
 *             "current": 11, // how many Sudoku objects are allocated
 *             // the function which called new
 *             "function": "Sudoku::successors() const",
 *             "peak": 12, // the peak number of Sudoku's allocated
 *             "size": 48 // the sizeof(Sudoku)
 *         }
 *     }
 * },
 * "int": {
 *     "4": {
 *         "16": {
 *             "array": 4,
 *             "current": 0,
 *             "function": "__gnu_cxx::new_allocator<int>::allocate(unsigned long, void const*)",
 *             "peak": 1,
 *             "size": 4
 *         }
 *     }
 * }
 * ...
 * ```
 */
class AllocCollector {
public:
    AllocCollector();
    /**
     * Adds an allocation's metadata to the list of allocated objects.
     * @param t_size the size of the allocation
     * @param t_align the alignment of the allocation
     * @param t_name the name of the type allocated
     * @param t_baseSize the sizeof of the type allocated
     * @param t_funcName the name of the function which allocated the type
     * @param t_ptr the pointer to the allocated type
     */
    void addObject(size_t t_size, size_t t_align,
                   const char* t_name, size_t t_baseSize,
                   const char* t_funcName, void* t_ptr);
    /**
     * Removes the given allocated type.
     * @param t_ptr the pointer to the allocated type
     */
    void removeObject(void* t_ptr);
    /**
     * Records the state of allocations in a JSON.
     */
    void takeSnapshot();
    virtual ~AllocCollector();
private:
    using json = nlohmann::basic_json<std::map, std::vector, std::string,
                                  bool, std::int64_t, std::uint64_t,
                                  double, mallocator>;
    std::ofstream m_objectsFile;
    std::map<std::string, AllocatedObject, std::less<std::string>,
             mallocator<std::pair<const std::string, AllocatedObject>>> m_allocObj;
    std::map<void*, Object*, std::less<void*>,
             mallocator<std::pair<const void*, Object*>>> m_obj;
    json m_snapshots;
    std::mutex m_mapLock;
    std::condition_variable m_cv;
    std::thread m_snapshotThread;
    size_t m_snapshotCount;
    bool m_waitingToPrint, m_threadStarted;

    void run();
};

#endif // __ALLOC_COLLECTOR_H__
