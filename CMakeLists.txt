cmake_minimum_required(VERSION 2.8.9)

project(efficient_pools)

option(BUILD_TESTS "Whether or not to build tests." ON)
option(BUILD_BENCHMARKS "Build benchmarks folder." OFF)
option(BUILD_EXAMPLES "Build examples folder." OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Werror -pthread -std=c++11 ${CMAKE_CXX_FLAGS}")

set(SRC ${PROJECT_SOURCE_DIR}/src)
set(INC ${PROJECT_SOURCE_DIR}/include)
set(LIBS ${PROJECT_SOURCE_DIR}/libs)

# src libs
include_directories(${INC})
add_subdirectory(${SRC})

# LLVM for custom_new_delete pass
find_package(LLVM 5.0 CONFIG)
if(LLVM_FOUND)
  add_definitions(${LLVM_DEFINITIONS})
  include_directories(${LLVM_INCLUDE_DIRS})
  add_subdirectory(llvm)
endif()

# test folder preparation
if(BUILD_TESTS)
  # Prepare "Catch" library for other executables
  set(CATCH_INCLUDE_DIR ${LIBS}/Catch2/single_include)
  add_library(Catch INTERFACE)
  target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

  enable_testing()
  include_directories(tests)
  add_subdirectory(tests)
endif()

# benchmarks folder preparation
if(BUILD_BENCHMARKS)
  find_package(Boost)
  if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
  endif()
  include_directories(${LIBS}/json/single_include)
  add_subdirectory(benchmarks)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
