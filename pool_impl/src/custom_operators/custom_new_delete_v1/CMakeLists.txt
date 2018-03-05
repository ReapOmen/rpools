# Prepare "libcustomnew.so"  for inject script and LLVMCustomNewPass
add_library(customnew SHARED
  ${SRC}/tools/LMLock.cpp
  ${SRC}/custom_operators/custom_new_delete_v1/GlobalPools.cpp
  ${SRC}/custom_operators/custom_new_delete_v1/custom_new_delete.cpp)
target_link_libraries(customnew linkedpools)
install(TARGETS customnew DESTINATION /usr/local/lib)
