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

file(GLOB SOURCES "src/*.cpp")

add_executable(test main.cpp ${SOURCES})
add_executable(benchmark_obj2 benchmark_Obj2.cpp ${SOURCES})
