#include <cxxabi.h>

#include "common.h"

using namespace custom_pass;
using std::string;

/**
 *  @param t_name a demangled Function name
 *  @return whether `t_name` is in `NEW_OPS` or in `NEW_NO_THROW_OPS`.
 */
bool custom_pass::isNew(const string& t_name) {
  return find(NEW_OPS.begin(), NEW_OPS.end(), t_name) != NEW_OPS.end()
    || find(NEW_NO_THROW_OPS.begin(), NEW_NO_THROW_OPS.end(), t_name)
      != NEW_NO_THROW_OPS.end();
}

/**
 *  @param t_name a demangled Function name
 *  @return whether `t_name` is in `DELETE_OPS` or in `DELETE_NO_THROW_OPS`.
 */
bool custom_pass::isDelete(const string& t_name) {
  return find(DELETE_OPS.begin(), DELETE_OPS.end(), t_name)
      != DELETE_OPS.end()
      || find(DELETE_NO_THROW_OPS.begin(), DELETE_NO_THROW_OPS.end(), t_name)
      != DELETE_NO_THROW_OPS.end();
}

/**
 *  @param t_name a demangled Function name
 *  @return the demangled name of a LLVM function.
 */
std::string custom_pass::getDemangledName(const string& t_name) {
  int status = -1;
  char* demangledName = abi::__cxa_demangle(
    t_name.c_str(), NULL, NULL, &status
  );
  std::string s(demangledName ? demangledName : "");
  return s;
}
