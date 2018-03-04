#include <cxxabi.h>
#include <algorithm>

#include "common.h"

using namespace custom_pass;
using std::string;

bool custom_pass::isNew(const string& t_name) {
  return find(NEW_OPS.begin(), NEW_OPS.end(), t_name) != NEW_OPS.end()
    || find(NEW_NO_THROW_OPS.begin(), NEW_NO_THROW_OPS.end(), t_name)
      != NEW_NO_THROW_OPS.end();
}

bool custom_pass::isDelete(const string& t_name) {
  return find(DELETE_OPS.begin(), DELETE_OPS.end(), t_name)
      != DELETE_OPS.end()
      || find(DELETE_NO_THROW_OPS.begin(), DELETE_NO_THROW_OPS.end(), t_name)
      != DELETE_NO_THROW_OPS.end();
}

std::string custom_pass::getDemangledName(const string& t_name) {
  int status = -1;
  char* demangledName = abi::__cxa_demangle(
    t_name.c_str(), nullptr, nullptr, &status
  );
  return demangledName ? demangledName : t_name;
}

// adapted from
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
void custom_pass::rtrim(std::string &s) {
    s.erase(s.find_last_not_of(' ') + 1);
}
