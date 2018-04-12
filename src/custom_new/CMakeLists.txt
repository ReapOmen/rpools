# Prepare "libcustomnew.so"  for inject script and LLVMCustomNewPass
add_library(customnew SHARED
  ${SRC}/tools/LMLock.cpp
  ${SRC}/custom_new/GlobalPools.cpp
  ${SRC}/custom_new/custom_new_delete.cpp)
target_link_libraries(customnew linkedpools)
install(TARGETS customnew DESTINATION lib)

# Prepare "libcustomnewdebug.so" for LLVMCustomNewPassDebug
add_library(customnewdebug SHARED
  ${SRC}/custom_new/AllocCollector.cpp
  ${SRC}/custom_new/custom_new_delete_debug.cpp)
target_include_directories(customnewdebug PRIVATE ${LIBS}/json/single_include)
install(TARGETS customnewdebug DESTINATION lib)
