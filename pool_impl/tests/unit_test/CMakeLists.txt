# Make test executable
add_executable(test_linked_pool test_linked_pool.cpp)
target_link_libraries(test_linked_pool Catch)

add_test(NAME TestLinkedPool COMMAND test_linked_pool)
