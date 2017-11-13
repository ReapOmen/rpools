#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <fstream>

std::string getPeakHeapUsage() {
    auto pid =  getpid();
    std::ifstream f("/proc/" + std::to_string(pid) + "/status");
    std::string temp = "";
    while(std::getline(f, temp)) {
        if (temp.substr(0, 6) == "VmPeak") {
            break;
        }
    }
    return temp;
}
