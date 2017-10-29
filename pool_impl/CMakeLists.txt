cmake_minimum_required(VERSION 2.8.9)

project(test_project)

add_subdirectory(src)

set (CMAKE_CXX_STANDARD 11)

if(LINUX)
  set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-declarations -Wno-reorder")
  if(CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

add_executable(test_bit bitpool_main.cpp)
add_executable(test_linked linkedpool_main.cpp)
add_executable(test_all main.cpp)
add_executable(test_reg_vs_linked reg_vs_linked.cpp)
