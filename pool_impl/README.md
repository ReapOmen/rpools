# pool_impl

## Build commands:

* `cd benchmarks && python3 generate_alloc_file.py && cd ..` -
to include allocation benchmarks (**optional**)
* `mkdir build && cd build && cmake .. && make` - to build the project


## plot_linked_tests.py

This is used to plot the different outputs of the executables from `benchmarks/linked_pool/`.

Usage (make sure you build the project first):
* `python3 plot_linked_tests.py -h` to see the help menu

Examples:
* `python3 plot_linked_tests.py -t ./build/benchmarks/linked_pool/bench_random -n 100000`

Explanation (can also be found in the help menu of the script):
* `./build/benchmarks/linked_pool/bench_random` - the executable that we want to execute
* `100000` - the upperbound of objects that will be created (in the case of `bench_worst`, the upperbound is multiplied by the size of the pool)


## plot_memory_usage

This is used to plot the output of the command:
`valgrind --tool=massif --stacks=yes --time-unit=ms`
on all the executables from `benchmarks/memory_usage/`.

Usage (make sure you build the project first):
* `python3 plot_memory_usage.py -h` to see the help menu


## inject_custom_new

This is a script which is used to run an executable with a custom new/delete
implementation. The custom new/delete library implements these operators by
using the `linked_pool/GlobalLinkedPool` class to (de)allocate small objects and
`malloc` to (de)allocate large objects.

If `-d` is supplied, at the end of the execution, the custom library will output
the memory used by the process to a file called `memory_snapshots_<PID>.output`,
and the number of objects in use to `object_snapshots_<PID>.output`.
The library takes snapshots every 10 ms.

Usage (make sure `libcustomnew.so` is present in your build directory):
* `inject_custom_new "my_exec args1 args2"` - to run your executable with the
custom implementation
* `inject_custom_new "my_exec args1 args2" -d` - to also generate debugging info
(this will be slower than the first command)

Note: Make sure you run the script from this folder!

## benchmarks/generate_alloc_file.py

This is a script that will generate allocation benchmarks. By running this
script, the `CMakeLists.txt` file will change and next time `make` is called,
the benchmarks will be compiled.

Note: Make sure you run this script from the `benchmarks` folder.

## time_alloc_benchmarks.py

This is a script which is used to run all the allocation benchmarks.

Usage:
* `python3 time_alloc_benchmarks.py -h` for instructions.\

Note: Make sure `generate_alloc_file.py` was run and the project was built.


## Licenses!
Everything is GPLv3 except for the following sources which have their own license:
* `src/avltree/avltree.c` (check source)
* `src/avltree/avltree.h` (check source)
* `src/pool_allocators/MemoryPool.h` (MIT)
