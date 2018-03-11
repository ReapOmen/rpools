## libLLVMCustomNewPass.so

This is a pass which replaces `new` and `delete` calls with `custom_new`
and `custom_delete`. In order to run it you will need `clang 5.0`.

## Usage

`clang++ -Xclang -load -Xclang /path/to/libLLVMCustomNewPass.so -o
hello /path/to/hello.cpp -lcustomnew` will compile `hello.cpp` with `clang`
and run the `CustomNewPass` pass on the file

If you have a `CMake` project, then you can achieve the above by:

```
# sets the appropriate flags for clang
set(CXX_FLAGS "-Xclang -load -Xclang /path/to/libLLVMCustomNewPass.so ${CXX_FLAGS}")
# links libcustomnew.so because without it your executable will complain
# about undefined references
target_link_libraries(your_executable customnew)
```

Make sure `cmake` is using `clang` to compile your project.

## libLLVMCustomNewPassDebug.so

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

To use it in a `CMake` project, check out how its done for `libLLVMCustomNewPass.so`,
and change `libLLVMCustomNewPass -> libLLVMCustomNewPassDebug` and
`customnew -> customnewdebug`.

