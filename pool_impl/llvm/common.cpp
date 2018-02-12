#include <cxxabi.h>

#include "common.h"

using namespace custom_pass;
using llvm::StringRef;

/**
 *  @param t_name a demangled Function name
 *  @return whether `t_name` is in `NEW_OPS` or in `NEW_NO_THROW_OPS`.
 */
bool custom_pass::isNew(const StringRef& t_name) {
  return find(NEW_OPS.begin(), NEW_OPS.end(), t_name) != NEW_OPS.end()
    || find(NEW_NO_THROW_OPS.begin(), NEW_NO_THROW_OPS.end(), t_name)
      != NEW_NO_THROW_OPS.end();
}

/**
 *  @param t_name a demangled Function name
 *  @return whether `t_name` is in `DELETE_OPS` or in `DELETE_NO_THROW_OPS`.
 */
bool custom_pass::isDelete(const StringRef& t_name) {
  return find(DELETE_OPS.begin(), DELETE_OPS.end(), t_name)
      != DELETE_OPS.end()
      || find(DELETE_NO_THROW_OPS.begin(), DELETE_NO_THROW_OPS.end(), t_name)
      != DELETE_NO_THROW_OPS.end();
}

/**
 *  @param t_name a demangled Function name
 *  @return the demangled name of a LLVM function.
 */
StringRef custom_pass::getDemangledName(const StringRef& t_name) {
  int status = -1;
  char* demangledName = abi::__cxa_demangle(
    t_name.str().c_str(), NULL, NULL, &status
  );
  StringRef s(demangledName ? demangledName : "");
  return s;
}
