# compile test_runner.cpp
add_library(testrunner STATIC test_runner.cpp)
target_link_libraries(testrunner PUBLIC Catch)

# test LinkedPool
add_executable(test_linked_pool test_linked_pool.cpp)
target_link_libraries(test_linked_pool PRIVATE linkedpools testrunner)
add_test(NAME TestLinkedPool COMMAND test_linked_pool)

# test GlobalLinkedPool
add_executable(test_global_linked_pool test_global_linked_pool.cpp)
target_link_libraries(test_global_linked_pool PRIVATE linkedpools testrunner)
add_test(NAME TestGlobalLinkedPool COMMAND test_global_linked_pool)

# test custom_new_delete.cpp
add_executable(test_custom_new_delete
  ${SRC}/tools/LMLock.cpp
  ${SRC}/custom_new/GlobalPools.cpp
  ${SRC}/custom_new/custom_new_delete.cpp
  test_custom_new_delete.cpp)
target_link_libraries(test_custom_new_delete PRIVATE linkedpools testrunner)
add_test(NAME TestCustomNewDelete COMMAND test_custom_new_delete)
