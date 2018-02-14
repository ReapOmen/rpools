add_library(LLVMCustomNewPass MODULE
  common.cpp
  CustomNewDelete.cpp)
add_library(LLVMCustomNewPassDebug MODULE
  common.cpp
  CustomNewDeleteDebug.cpp)

install(TARGETS LLVMCustomNewPass DESTINATION /usr/local/lib)
