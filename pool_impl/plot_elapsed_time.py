#!/usr/bin/python3

import json
import subprocess
import matplotlib.pyplot as plt


def get_json(json_file):
    """
    Return the JSON found in the given file.
    :param json_file: the file which contains the JSON
    :type json_file: str
    :returns: dict
    """
    with open(json_file) as f:
        content = json.load(f)
    return content


def plot(executable, json_file, limit):
    """
    Run <executable> 10 times in increments of <limit> / 10.
    :param executable: the path to the executable
    :type executable: str
    :param json_file: the file which contains the JSON
    :type json_file: str
    :param limit: the maximum number passed to the executable
    :type limit: int
    """
    # if the (de)alloc_time_plot lists were initialised or not
    is_init = False
    # range of values with which the benchmark is called
    alloc_range = range(limit//10, limit+1, limit//10)
    for i in alloc_range:
        print("\r", i, "/", limit, end="")
        # calls the benchmark <executable> with the argument <i>
        subprocess.run([executable, str(i)])
        # the executable will create a file denoted by <json_file>
        bench_results = get_json(json_file)
        if not is_init:
            num_of_implementations = len(bench_results["allocators"])
            # each sub array contains the allocation times for one
            # specific allocator
            # e.g. subarray 1 contains the allocation times of new/delete
            # for 10, 20, 30, ..., 100 objects; subarray 2 contains the same,
            # but for LinkedPool
            alloc_time_plot = [[] for i in range(num_of_implementations)]
            dealloc_time_plot = [[] for i in range(num_of_implementations)]
            # the names of all allocators
            labels = [name for name in bench_results["allocators"]]
            is_init = True
        k = 0
        for name, allocator in bench_results["allocators"].items():
            alloc_time_plot[k].append(allocator["allocation_time"])
            dealloc_time_plot[k].append(allocator["deallocation_time"])
            k += 1
    print()
    plot_time(alloc_range, alloc_time_plot, 211,
              'Allocation time of the %d implementations' %
              (num_of_implementations), labels)
    plot_time(alloc_range, dealloc_time_plot, 212,
              'Deallocation time of the %d implementations' %
              (num_of_implementations), labels)


def plot_time(x, y, subplot, title, labels):
    """
    Plot the given information.
    :param x: the X axis
    :type x: list
    :param y: the Y axis
    :type y: list
    :param subplot: the subplot to use
    :type subplot: int
    :param title: the title of the plot
    :type title: str
    :param labels: the labels of the plot
    :type labels: list
    """
    plt.subplot(subplot)
    plt.title(title)
    plots = []
    for i in range(0, len(labels)):
        plots.append(plt.plot(x, y[i], label=labels[i]))
    plt.xlabel('Number of allocations')
    plt.ylabel('ms')
    plt.legend()


if __name__ == "__main__":
    import os
    import argparse
    parser = argparse.ArgumentParser(description='Plot test data')
    parser.add_argument('--benchmark', '-b', help='Which benchmark to run '
                        '(default: bench_normal)',
                        default="./build/benchmarks/elapsed_time/bench_normal")
    parser.add_argument('--upper-bound', '-n',
                        help='The upperbound of the number of '
                        'allocations (default: 100000)',
                        type=int, default=100000)
    args = parser.parse_args()
    json_file = args.benchmark.split(os.path.sep)[-1].split('_')[-1] + \
        '_time_taken.json'
    plot(args.benchmark, json_file, args.upper_bound)
    plt.show()
