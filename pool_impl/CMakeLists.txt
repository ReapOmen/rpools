cmake_minimum_required(VERSION 2.8.9)

project(test_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -pthread -std=c++11 ${CMAKE_CXX_FLAGS}")

set(SRC ${PROJECT_SOURCE_DIR}/src)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_subdirectory(src)
include_directories(${SRC})

# Prepare "avltree" library for linking
add_library(avltree
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c)
set_target_properties(avltree PROPERTIES PUBLIC_HEADER ${SRC}/avltree/avl_utils.h)
target_compile_options(avltree PRIVATE -O3)

# Prepare "gllinkedpool" library for linking
add_library(gllinkedpool SHARED
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c
  ${SRC}/pool_allocators/GlobalLinkedPool.cpp)
set_target_properties(gllinkedpool PROPERTIES PUBLIC_HEADER
  ${SRC}/pool_allocators/GlobalLinkedPool.h)
target_compile_options(gllinkedpool PRIVATE -O3)

# Prepare "nsgllinkedpool" library for linking
add_library(nsgllinkedpool SHARED
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c
  ${SRC}/pool_allocators/NSGlobalLinkedPool.cpp)
set_target_properties(nsgllinkedpool PROPERTIES PUBLIC_HEADER
  ${SRC}/pool_allocators/NSGlobalLinkedPool.h)
target_compile_options(nsgllinkedpool PRIVATE -O3)

# Prepare "globalpools" library for linking
add_library(globalpools SHARED
  ${SRC}/custom_operators/custom_new_delete_v1/GlobalPools.cpp)
target_link_libraries(globalpools PUBLIC nsgllinkedpool)
target_compile_options(globalpools PRIVATE -O3)

# Prepare "libcustomnew.so" for inject script
add_library(customnew SHARED
  ${SRC}/custom_operators/custom_new_delete_v1/custom_new_delete.cpp)
target_link_libraries(customnew PRIVATE globalpools)
set_target_properties(customnew PROPERTIES PUBLIC_HEADER
  ${SRC}/custom_operators/custom_new_delete_v1/custom_new_delete.h)
target_compile_options(customnew PRIVATE -O3)

# Prepare "libcustomnewdebug.so" for inject script
add_library(customnewdebug SHARED
  ${SRC}/custom_operators/debug/custom_new_delete_debug.cpp
  ${SRC}/tools/AllocCollector.cpp)
target_link_libraries(customnewdebug PRIVATE gllinkedpool)

find_package(LLVM 3.8 CONFIG)
if(LLVM_FOUND)
  add_definitions(${LLVM_DEFINITIONS})
  include_directories(${LLVM_INCLUDE_DIRS})
  add_subdirectory(llvm)
endif()

# Prepare "libcustomnew2.so" for inject script
add_library(customnew2 SHARED
  ${SRC}/custom_operators/custom_new_delete_v2/custom_new_delete.cpp)
target_link_libraries(customnew2 PRIVATE gllinkedpool)
target_compile_options(customnew2 PRIVATE -O3)

enable_testing()
add_subdirectory(tests)
include_directories(./tests)

add_subdirectory(benchmarks)
