#include <ctime>
#include <fstream>

/**
 *  Writes a certain message to a given file.
 *  @param f - the file in which the message is written
 *  @nameOfobject - the name of the object printed
 *  @timeTaken - the number of ms it took to (de)allocate the object
 *  @isDealloc - is it a deallocation or an allocation
 *  @name - the name of the allocator
 */
void printToFile2(std::ofstream& f,
                 const std::string& nameOfObject,
                 float timeTaken,
                 bool isDealloc,
                 const std::string& name) {
    std::string alloc = isDealloc ? "Deallocate " : "Allocate ";
    f << alloc << nameOfObject << " " << name << ": "
      << timeTaken << " ms" << std::endl;
}

/**
 *  Writes a certain message to a given file.
 *  @param f - the file in which the message is written
 *  @nameOfobject - the name of the object printed
 *  @start - the starting time of the computation
 *  @isDealloc - is it a deallocation or an allocation
 *  @name - the name of the allocator
 */
void printToFile(std::ofstream& f,
                 const std::string& nameOfObject,
                 const std::clock_t& start,
                 bool isDealloc,
                 const std::string& name) {
    std::string alloc = isDealloc ? "Deallocate " : "Allocate ";
    float timeTaken = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
    f << alloc << nameOfObject << " " << name << ": "
      << timeTaken << " ms" << std::endl;
}
