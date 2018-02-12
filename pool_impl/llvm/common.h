#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <llvm/ADT/StringRef.h>

namespace custom_pass {

const std::vector<llvm::StringRef> NEW_OPS = {
  "operator new(unsigned long)",
  "operator new[](unsigned long)"
};
const std::vector<llvm::StringRef> NEW_NO_THROW_OPS = {
  "operator new(unsigned long, std::nothrow_t const&)",
  "operator new[](unsigned long, std::nothrow_t const&)"
};
const std::vector<llvm::StringRef> DELETE_OPS = {
  "operator delete(void*)",
  "operator delete[](void*)"
};
const std::vector<llvm::StringRef> DELETE_NO_THROW_OPS = {
  "operator delete(void*, std::nothrow_t const&)",
  "operator delete[](void*, std::nothrow_t const&)"
};

bool isNew(const llvm::StringRef& t_name);
bool isDelete(const llvm::StringRef& t_name);
llvm::StringRef getDemangledName(const llvm::StringRef& t_name);
}
#endif // __COMMON_H__
