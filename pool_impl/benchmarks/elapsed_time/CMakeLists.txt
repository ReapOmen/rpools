if(Boost_FOUND)
  set_source_files_properties(bench_normal_order.cpp
    PROPERTIES COMPILE_DEFINITIONS INCLUDE_BOOST=1)
  set_source_files_properties(bench_worst_order.cpp
    PROPERTIES COMPILE_DEFINITIONS INCLUDE_BOOST=1)
  set_source_files_properties(bench_random_order.cpp
    PROPERTIES COMPILE_DEFINITIONS INCLUDE_BOOST=1)
  set_source_files_properties(bench_random2_order.cpp
    PROPERTIES COMPILE_DEFINITIONS INCLUDE_BOOST=1)
  set_source_files_properties(bench_specified_order.cpp
    PROPERTIES COMPILE_DEFINITIONS INCLUDE_BOOST=1)
endif()

add_executable(bench_normal bench_normal_order.cpp)
target_link_libraries(bench_normal avltree)
add_executable(bench_worst bench_worst_order.cpp)
target_link_libraries(bench_worst avltree)
add_executable(bench_random bench_random_order.cpp)
target_link_libraries(bench_random avltree)
add_executable(bench_specified bench_specified_order.cpp)
target_link_libraries(bench_specified avltree)
add_executable(bench_random2 bench_random2_order.cpp)
target_link_libraries(bench_random2 avltree)
