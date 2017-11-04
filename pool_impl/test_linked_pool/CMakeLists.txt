set(LP_PATH ${PROJECT_SOURCE_DIR}/src)
include_directories(${LP_PATH})

add_executable(test_normal test_normal_order.cpp)
add_executable(test_worst test_worst_order.cpp)
add_executable(test_random test_random_order.cpp)
add_executable(test_specified test_specified_order.cpp)
add_executable(test_random2 test_random2_order.cpp)
