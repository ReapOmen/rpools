#include <ctime>
#include <fstream>

void printToFile(std::ofstream& f,
                 const std::string& nameOfObject,
                 const std::clock_t& start,
                 bool isDealloc,
                 bool isLinked) {
    std::string alloc = isDealloc ? "Deallocate " : "Allocate ";
    std::string type = isLinked ? " with LinkedPool: " : " normally: ";
    f << alloc << nameOfObject << type
      << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000)
      << " ms" << std::endl;
}
