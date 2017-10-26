# test_proj

The aim of this project is to show how pools might help with decreasing
memory usage.


The notation `SomeObject/2/3(P)` is equivalent to `SomeObject`, `SomeObject2`
and `SomeObject3` with a pool of size `P`.


`main.cpp` does the following:
* it takes 1-2 command line arguments to specify (in order):
    * how many `SomeObject/2/3`s and `Obj1/2/3`s to allocate (`N`)
    * the pool size of `SomeObject3`(`P`)
* calculates the time it takes to allocate and deallocate `N` objects
of type `SomeObject/2/3(P)` and `Obj1/2/3`s
* creates a few files with the following information:
    * `time_taken.txt` - which contains the time calculated above
    * `overheads.txt` - which approximates the overheads of all 5 implementations
    * `allocation_file.txt` - which tracks how many allocations of size `T` have been made
    * `deallocation_file.txt` - which tracks how many deallocations of size `T` have been made


To run `test_proj`, you will need to following:
* run `mkdir build && cd build && cmake .. && make` to compile the project
* run `./test <number_of_allocations> [<pool_size>]`


`graphs.py` is a python script that creates charts of the above info. It is used
to see how the different implementations behave (i.e. how much memory overheads they use,
how much time it takes to allocate/deallocate objects, etc.)

It creates charts by varying the number of the allocations command
line argument of `test_proj`.


To see the graphs you will need to:
* compile `test_proj` as shown above
* run `python3 graphs.py`

## Dependencies:
* for `test_proj`:
  * `CMake 2.8.4+`
* for `graphs.py`:
  * `matplotlib`
