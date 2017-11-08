cmake_minimum_required(VERSION 2.8.9)

project(test_project)

set(CMAKE_CXX_STANDARD 11)
enable_testing()

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

set(LP_PATH ${PROJECT_SOURCE_DIR}/src)
include_directories(${LP_PATH})
include_directories(${PROJECT_SOURCE_DIR}/tests)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmarks)

if(LINUX)
  set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Werr")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-declarations -Wno-reorder")
  if(CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

add_executable(test_bit bitpool_main.cpp)
add_executable(test_linked linkedpool_main.cpp)
add_executable(test_all main.cpp)
