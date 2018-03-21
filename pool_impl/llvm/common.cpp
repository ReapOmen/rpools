#include <cxxabi.h>
#include <algorithm>

#include "common.hpp"

using namespace custom_pass;
using namespace llvm;
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

size_t custom_pass::getAlignment(const DataLayout& dataLayout, Type* type) {
  if (type->isStructTy()) {
    auto sType = dyn_cast<StructType>(type);
    size_t maxAlignment = 1;
    for (auto subType : sType->elements()) {
      size_t alignment = custom_pass::getAlignment(dataLayout, subType);
      if (alignment == alignof(std::max_align_t)) {
        return alignment;
      }
      maxAlignment = std::max(maxAlignment, alignment);
    }
    return maxAlignment;
  } else if (type->isArrayTy() || type->isVectorTy()) {
    auto subType = dyn_cast<SequentialType>(type)->getElementType();
    return custom_pass::getAlignment(dataLayout, subType);
  } else {
    return dataLayout.getABITypeAlignment(type);
  }
}

// adapted from
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
void custom_pass::rtrim(std::string &s) {
    s.erase(s.find_last_not_of(' ') + 1);
}
