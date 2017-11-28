#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <thread>
#include <iostream>
using std::cout;
using std::endl;



static auto pid =  getpid();

std::string __getStatusField(const std::string& field) {
    std::ifstream f("/proc/" + std::to_string(pid) + "/status");

    std::string temp = "";
    while(std::getline(f, temp)) {
        if (temp.substr(0, field.size()) == field) {
            break;
        }
    }
    std::string num = "";
    for (char c : temp) {
        if (c >= '0' && c <= '9') {
            num += c;
        }
    }
    return num;
}

std::string getHeapUsage() {
    return __getStatusField("VmSize");
}

std::string getStackUsage() {
    return __getStatusField("VmStk");
}

std::string getPeakHeapUsage() {
    return __getStatusField("VmPeak");
}
