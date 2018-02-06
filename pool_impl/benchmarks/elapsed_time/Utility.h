#include <ctime>
#include <fstream>
#include "nlohmann/json.hpp"

/**
 *  Represents a class which is responsible for holding a json object
 *  which is dumped to a given file when it is destroyed.
 *  This class is used by benchmarks in order to create files in which
 *  the total (de)allocation time of several allocators is recorded.
 *  @par
 *  This is a sample of what this class might dump to a file:
 *  ```
 *   {
 *       "allocators": {
 *           "LinkedPool": {
 *               "allocation_time": 0.6010000109672546,
 *               "deallocation_time": 0.4350000023841858
 *           },
 *           "LinkedPool3": {
 *               "allocation_time": 0.45500001311302185,
 *               "deallocation_time": 0.4339999854564667
 *           },
 *           "MemoryPool": {
 *               "allocation_time": 0.2639999985694885,
 *               "deallocation_time": 0.26600000262260437
 *           },
 *           "boost::object_pool": {
 *               "allocation_time": 0.4189999997615814,
 *               "deallocation_time": 604.7899780273438
 *           },
 *           "new/delete": {
 *               "allocation_time": 0.5070000290870667,
 *               "deallocation_time": 0.3050000071525574
 *           }
 *       },
 *       "number_of_allocations": 17000
 *   }
 *  ```
 */
struct JSONWriter {
    std::ofstream f;
    nlohmann::json j;

    /**
     *  @param t_fileName the file in which the json will be dumped
     *  @param t_numOfAllocs the value of the key `number_of_allocations`
     */
    JSONWriter(const std::string& t_fileName, size_t t_numOfAllocs)
        : f(t_fileName), j() {
        j["number_of_allocations"] = t_numOfAllocs;
    }

    /**
     *  Adds an allocation entry.
     *  @param t_allocatorName the name of the allocator
     *  @param t_timeTaken the time it took to allocate the objects
     */
    void addAllocation(const std::string& t_allocatorName,
                       float t_timeTaken) {
        j["allocators"][t_allocatorName]["allocation_time"] = t_timeTaken;
    }

    /**
     *  Adds an allocation entry.
     *  @param t_allocatorName the name of the allocator
     *  @param t_start the starting time of the allocation
     */
    void addAllocation(const std::string& t_allocatorName,
                       const std::clock_t& t_start) {
        float timeTaken = (std::clock() - t_start) /
            (double)(CLOCKS_PER_SEC / 1000);
        j["allocators"][t_allocatorName]["allocation_time"] = timeTaken;
    }

    /**
     *  Adds an deallocation entry.
     *  @param t_allocatorName the name of the allocator
     *  @param t_timeTaken the time it took to deallocate the objects
     */
    void addDeallocation(const std::string& t_allocatorName,
                         float t_timeTaken) {
        j["allocators"][t_allocatorName]["deallocation_time"] = t_timeTaken;
    }

    /**
     *  Adds an deallocation entry.
     *  @param t_allocatorName the name of the allocator
     *  @param t_start the starting time of the deallocation
     */
    void addDeallocation(const std::string& t_allocatorName,
                         const std::clock_t& t_start) {
        float timeTaken = (std::clock() - t_start) /
            (double)(CLOCKS_PER_SEC / 1000);
        j["allocators"][t_allocatorName]["deallocation_time"] = timeTaken;
    }

    virtual ~JSONWriter() {
        f << j.dump(4);
    }
};
