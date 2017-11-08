#!/usr/bin/python3

import subprocess
import matplotlib.pyplot as plt


NUM_OF_IMPLEMENTATIONS = 0
FILE = ''
EXEC = ''
LIMIT = 100000
labels = set()


def get_alloc_time():
    lst = []
    num = 0
    with open(FILE) as f:
        for line in f:
            if not line.endswith('.\n'):
                split = line.split(' ')
                labels.add(split[2][0:-1])
                lst.append(float(split[-2]))
            else:
                num = int(line.split(' ')[1])
    return (num, lst)


def call_test(allocs):
    subprocess.run([EXEC, str(allocs)])


def plot():
    global labels, NUM_OF_IMPLEMENTATIONS
    alloc_time_plot = []
    dealloc_time_plot = []
    alloc_range = range(LIMIT//10, LIMIT+1, LIMIT//10)
    plot_alloc_range = []
    is_init = False
    for i in alloc_range:
        print("\r", i, "/", LIMIT, end="")
        call_test(i)
        alloc_time = get_alloc_time()
        plot_alloc_range.append(alloc_time[0])
        if not is_init:
            NUM_OF_IMPLEMENTATIONS = len(alloc_time[1]) // 2
            alloc_time_plot = [[] for i in range(NUM_OF_IMPLEMENTATIONS)]
            dealloc_time_plot = [[] for i in range(NUM_OF_IMPLEMENTATIONS)]
            is_init = True
        for j in range(0, NUM_OF_IMPLEMENTATIONS):
            index = j * 2
            alloc_time_plot[j].append(alloc_time[1][index])
            dealloc_time_plot[j].append(alloc_time[1][index + 1])
    print()
    labels = list(labels)
    labels.sort()
    plot_time(plot_alloc_range, alloc_time_plot, 211,
              'Allocation time of the %d implementations' %
              (NUM_OF_IMPLEMENTATIONS))
    plot_time(plot_alloc_range, dealloc_time_plot, 212,
              'Deallocation time of the %d implementations' %
              (NUM_OF_IMPLEMENTATIONS))


def plot_time(x, y, subplot, title):
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
    parser.add_argument('--test', '-t', help='Which test to run '
                        '(default: test_normal)',
                        default="./build/test_linked_pool/test_normal")
    parser.add_argument('--upper-bound', '-n',
                        help='The upperbound of the number of '
                        'allocations (default: 100000)',
                        type=int, default=100000)
    args = parser.parse_args()
    EXEC = args.test
    FILE = EXEC.split(os.path.sep)[-1].split('_')[-1] + '_time_taken.txt'
    LIMIT = args.upper_bound
    plot()
    plt.show()
