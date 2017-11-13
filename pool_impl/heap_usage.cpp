#include "src/Tools.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
#include "src/custom_new_delete.h"

int main() {
    cout << getPeakHeapUsage() << endl;
}
