# pool_impl

Build command:

`mkdir build && cd build && cmake .. && make`

# plot_linked_tests.py

This is used to plot the different outputs of the executables from `test_linked_pool`.

Usage (make sure you build the project first):
* `python3 plot_linked_tests.py -h` to see the help menu

Examples:
* `python3 plot_linked_tests.py -t ./build/test_linked_pool/test_random -n 100000`

Explanation (can also be found in the help menu of the script):
* `./build/test_linked_pool/test_random` - the executable that we want to execute
* `100000` - the upperbound of objects that will be created (in the case of `test_worst`, the upperbound is multiplied by `170`)
