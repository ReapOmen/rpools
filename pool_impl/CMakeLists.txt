cmake_minimum_required(VERSION 2.8.9)

project(test_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -pthread -std=c++11 ${CMAKE_CXX_FLAGS}")

enable_testing()

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# Prepare "avltree" library for linking
add_library(avltree ./src/avltree/avltree.c)
set_target_properties(avltree PROPERTIES PUBLIC_HEADER ./src/avltree/avl_utils.h)
target_include_directories(avltree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/avltree)
target_compile_options(avltree PRIVATE -O3)

set(LP_PATH ${PROJECT_SOURCE_DIR}/src)
include_directories(${LP_PATH})

add_subdirectory(src)

# Prepare "libcustomnewdebug.so" for inject script
add_library(customnewdebug SHARED
  ${LP_PATH}/avltree/avltree.c
  ${LP_PATH}/custom_new_delete_debug.cpp
  ${LP_PATH}/tools/AllocCollector.cpp)
target_include_directories(customnewdebug PRIVATE ${LP_PATH})
target_link_libraries(customnewdebug avltree)

# Prepare "libcustomnew.so" for inject script
add_library(customnew SHARED ${LP_PATH}/avltree/avltree.c ${LP_PATH}/custom_new_delete.cpp)
target_include_directories(customnew PRIVATE ${LP_PATH})
target_compile_options(customnew PRIVATE -O3)

# Prepare "libcustomnew2.so" for inject script
add_library(customnew2 SHARED ${LP_PATH}/avltree/avltree.c ${LP_PATH}/custom_new_delete2.cpp)
target_include_directories(customnew2 PRIVATE ${LP_PATH})
target_compile_options(customnew2 PRIVATE -O3)

add_subdirectory(tests)
include_directories(${PROJECT_SOURCE_DIR}/tests)

add_subdirectory(benchmarks)
