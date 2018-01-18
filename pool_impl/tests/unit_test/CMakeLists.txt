# compile test_runner.cpp
add_library(testrunner STATIC test_runner.cpp)
target_link_libraries(testrunner PUBLIC Catch)

# test LinkedPool1
add_executable(test_linked_pool test_linked_pool.cpp)
target_link_libraries(test_linked_pool PRIVATE avltree testrunner)
add_test(NAME TestLinkedPool COMMAND test_linked_pool)

# test LinkedPool3
add_executable(test_linked_pool3 test_linked_pool3.cpp)
target_link_libraries(test_linked_pool3 PRIVATE avltree testrunner)
add_test(NAME TestLinkedPool3 COMMAND test_linked_pool3)

# test GlobalLinkedPool
add_executable(test_global_linked_pool test_global_linked_pool.cpp)
target_link_libraries(test_global_linked_pool PRIVATE avltree testrunner)
add_test(NAME TestGlobalLinkedPool COMMAND test_global_linked_pool)

# test custom_new_delete.cpp
add_executable(test_custom_new_delete test_custom_new_delete.cpp)
target_link_libraries(test_custom_new_delete PRIVATE testrunner avltree)
add_test(NAME TestCustomNewDelete COMMAND test_custom_new_delete)

# test custom_new_delete2.cpp
add_executable(test_custom_new_delete2 test_custom_new_delete2.cpp)
target_link_libraries(test_custom_new_delete2 PRIVATE testrunner avltree)
add_test(NAME TestCustomNewDelete2 COMMAND test_custom_new_delete2)
