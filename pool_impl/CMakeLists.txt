cmake_minimum_required(VERSION 2.8.9)

project(test_project)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)
add_subdirectory(test_linked_pool)

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
