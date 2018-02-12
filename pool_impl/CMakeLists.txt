cmake_minimum_required(VERSION 2.8.9)

project(efficient_pools)

option(BUILD_TESTS "Whether or not to build tests." ON)
option(BUILD_BENCHMARKS "Build benchmarks folder." OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Werror -pthread -std=c++11 ${CMAKE_CXX_FLAGS}")

set(SRC ${PROJECT_SOURCE_DIR}/src)

# src libs
include_directories(${SRC})
add_subdirectory(${SRC})

# Prepare "gllinkedpool" library for linking
add_library(gllinkedpool SHARED
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c
  ${SRC}/tools/LMLock.cpp
  ${SRC}/pool_allocators/GlobalLinkedPool.cpp)
set_target_properties(gllinkedpool PROPERTIES PUBLIC_HEADER
  ${SRC}/pool_allocators/GlobalLinkedPool.h)

# Prepare "nsgllinkedpool" library for linking
add_library(nsgllinkedpool SHARED
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c
  ${SRC}/pool_allocators/NSGlobalLinkedPool.cpp)
set_target_properties(nsgllinkedpool PROPERTIES PUBLIC_HEADER
  ${SRC}/pool_allocators/NSGlobalLinkedPool.h)

# Prepare "globalpools" library for linking
add_library(globalpools SHARED
  ${SRC}/tools/LMLock.cpp
  ${SRC}/custom_operators/custom_new_delete_v1/GlobalPools.cpp)
target_link_libraries(globalpools PUBLIC nsgllinkedpool)

# Prepare "libcustomnew.so" for inject script
add_library(customnew SHARED
  ${SRC}/custom_operators/custom_new_delete_v1/custom_new_delete.cpp)
target_link_libraries(customnew PRIVATE globalpools)
set_target_properties(customnew PROPERTIES PUBLIC_HEADER
  ${SRC}/custom_operators/custom_new_delete_v1/custom_new_delete.h)

# Prepare "libcustomnewdebug.so" for inject script
add_library(customnewdebug SHARED
  ${SRC}/tools/AllocCollector.cpp)
target_include_directories(customnewdebug PRIVATE ./libs/json/single_include)

# Prepare "libcustomnew2.so" for inject script
add_library(customnew2 SHARED
  ${SRC}/custom_operators/custom_new_delete_v2/custom_new_delete.cpp)
target_link_libraries(customnew2 PRIVATE gllinkedpool)

# LLVM for custom_new_delete pass
find_package(LLVM 3.8 CONFIG)
if(LLVM_FOUND)
  add_definitions(${LLVM_DEFINITIONS})
  include_directories(${LLVM_INCLUDE_DIRS})
  add_subdirectory(llvm)
endif()

# test folder preparation
if(BUILD_TESTS)
  # Prepare "Catch" library for other executables
  set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/Catch2/single_include)
  add_library(Catch INTERFACE)
  target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

  enable_testing()
  include_directories(./tests)
  add_subdirectory(./tests)
endif()

# benchmarks folder preparation
if(BUILD_BENCHMARKS)
  find_package(Boost)
  if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
  endif()
  include_directories(./libs/json/single_include)
  add_subdirectory(benchmarks)
endif()
