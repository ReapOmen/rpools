# pool_impl

## Build commands:

* `cd benchmarks && python3 generate_alloc_file.py && cd ..` -
to include allocation benchmarks (**optional**)
* `mkdir build && cd build && cmake .. && make` - to build the project
* `make install` - to install the libraries
* `make test` - runs the tests of the project (this assumes that the project
is built)
* `pip install -r ./requirements.txt` will install all the dependencies of
all python scripts
* `clang++ -Xclang -load -Xclang /path/to/libLLVMCustomNewPass.so -o
hello /path/to/hello.cpp -lcustomnew` will compile `hello.cpp` with `clang`
and run a special pass on the file in which all calls to `operator new/delete`
will get replaced by `custom_new/delete`.

## LLVM CustomNewDeleteDebug pass

This pass collects allocation information (type name, type size, allocation size,
function name, etc.) during runtime by injecting
`libcustomnewdebug` into the compiled program. If the executable is run, an
`object_snapshots_<PID>.json` is generated.

`clang++ -Xclang -load -Xclang /path/to/libLLVMCustomNewDebugPass.so -o
hello /path/to/hello.cpp -lcustomnewdebug` will compile hello with
the debug pass.

The command `./generate_obj_alloc_html.py -f /path/to/object_snapshots_<PID>.json`
will generate an HTML file which will render the results into table format.

Make sure `cd debug && npm install` is run before generation.

## plot_elapsed_time.py

This is used to plot the different outputs of the executables from
`benchmarks/elapsed_time/`.

Usage (make sure you build the project first):
* `python3 plot_elapsed_time.py -h` to see the help menu

Examples:
* `python3 plot_elapsed_time.py -b ./build/benchmarks/elapsed_time/bench_random
-n 100000`

Explanation (can also be found in the help menu of the script):
* `./build/benchmarks/elapsed_time/bench_random` - the executable that we want
to execute
* `100000` - the upperbound of objects that will be created (in the case of
`bench_worst`, the upperbound is multiplied by the size of the pool)


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

Usage (make sure `libcustomnew.so` is present in your build directory):
* `inject_custom_new my_exec args1 args2` - to run your executable with the
custom implementation that is also thread safe

Note: Make sure you run the script from **this** folder!


## benchmarks/generate_alloc_file.py

This is a script that will generate allocation benchmarks. By running this
script, the `CMakeLists.txt` file will change and next time `make` is called,
the benchmarks will be compiled.

Note: Make sure you run this script from the `benchmarks` folder.


## time_alloc_benchmarks.py

This is a script which is used to run all the allocation benchmarks.

Usage:
* `python3 time_alloc_benchmarks.py -h` for instructions.

Note: Make sure `generate_alloc_file.py` was run and the project was built.


## Licenses!
Everything is GPLv3 except for the following files which have their own license:
* `include/tools/light_lock.h` (check source)
* `src/avltree/avltree.c` (check source)
* `include/avltree/avltree.h` (check source)
* `src/pool_allocators/MemoryPool.h` (MIT)
