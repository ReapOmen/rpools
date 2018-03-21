#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <string>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>

namespace custom_pass {

const std::vector<std::string> NEW_OPS = {
  "operator new(unsigned long)",
  "operator new[](unsigned long)"
};
const std::vector<std::string> NEW_NO_THROW_OPS = {
  "operator new(unsigned long, std::nothrow_t const&)",
  "operator new[](unsigned long, std::nothrow_t const&)"
};
const std::vector<std::string> DELETE_OPS = {
  "operator delete(void*)",
  "operator delete[](void*)"
};
const std::vector<std::string> DELETE_NO_THROW_OPS = {
  "operator delete(void*, std::nothrow_t const&)",
  "operator delete[](void*, std::nothrow_t const&)"
};

/**
 *  @param t_name a demangled Function name
 *  @return whether `t_name` is in `NEW_OPS` or in `NEW_NO_THROW_OPS`.
 */
bool isNew(const std::string& t_name);

/**
 *  @param t_name a demangled Function name
 *  @return whether `t_name` is in `DELETE_OPS` or in `DELETE_NO_THROW_OPS`.
 */
bool isDelete(const std::string& t_name);

/**
 *  @param t_name a demangled Function name
 *  @return the demangled name of a LLVM function.
 */
std::string getDemangledName(const std::string& t_name);

/**
 *  @param dataLayout the DataLayout of the module
 *  @param type the type whose alignment to be calculated
 *  @return the true alignment of the given type.
 */
size_t getAlignment(const llvm::DataLayout& dataLayout, llvm::Type* type);

/**
 * Erases the whitespace from the end of the string.
 * @param s the string from which to erase
 */
void rtrim(std::string &s);
}
#endif // __COMMON_H__
