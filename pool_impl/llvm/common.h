#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <string>
#include <llvm/ADT/StringRef.h>

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

bool isNew(const std::string& t_name);
bool isDelete(const std::string& t_name);
std::string getDemangledName(const std::string& t_name);
}
#endif // __COMMON_H__
