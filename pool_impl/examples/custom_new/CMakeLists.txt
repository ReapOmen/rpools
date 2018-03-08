add_executable(custom_new_example example.cpp)
# make sure to link libcustomnew
target_link_libraries(custom_new_example customnew)
