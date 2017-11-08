#include <ctime>
#include <fstream>

void printToFile(std::ofstream& f,
                 const std::string& nameOfObject,
                 const std::clock_t& start,
                 bool isDealloc,
                 const std::string& name) {
    printToFile(f, nameOfObject,
                (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000),
                isDealloc, name);
}

void printToFile(std::ofstream& f,
                 const std::string& nameOfObject,
                 float timeTaken,
                 bool isDealloc,
                 const std::string& name) {
    std::string alloc = isDealloc ? "Deallocate " : "Allocate ";
    f << alloc << nameOfObject << " " << name << ": "
      << timeTaken << " ms" << std::endl;
}
