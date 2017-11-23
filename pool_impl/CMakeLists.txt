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

set(CMAKE_CXX_FLAGS "-Wall -Werror -pthread -std=c++11 ${CMAKE_CXX_FLAGS}")

add_subdirectory(src)
add_library(customnew SHARED ./src/custom_new_delete.cpp ./src/AllocCollector.cpp)
target_include_directories(customnew PRIVATE ${LP_PATH})

add_subdirectory(tests)
add_subdirectory(benchmarks)
