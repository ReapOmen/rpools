# Prepare "libcustomnewdebug.so" for LLVMCustomNewPassDebug
add_library(customnewdebug SHARED
  ${SRC}/custom_operators/debug/AllocCollector.cpp
  ${SRC}/custom_operators/debug/custom_new_delete_debug.cpp)
target_include_directories(customnewdebug PRIVATE ${LIBS}/json/single_include)
install(TARGETS customnewdebug DESTINATION /usr/local/lib)
