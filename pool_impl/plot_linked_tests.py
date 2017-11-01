#!/usr/bin/python3

import subprocess
import matplotlib.pyplot as plt


NUM_OF_IMPLEMENTATIONS = 2
FILE = "./time_taken.txt"
EXEC = "./build/test_linked_pool/test_normal"
LIMIT = 100000
labels = ["Regular", "LinkedPool"]


def get_alloc_time():
    lst = []
    num = 0
    with open(FILE) as f:
        for line in f:
            if not line.endswith('.\n'):
                lst.append(float(line.split(' ')[-2]))
            else:
                num = int(line.split(' ')[1])
    return (num, lst)


def call_test(allocs):
    subprocess.run([EXEC, str(allocs)])


def plot():
    alloc_time_plot = [[] for i in range(NUM_OF_IMPLEMENTATIONS)]
    dealloc_time_plot = [[] for i in range(NUM_OF_IMPLEMENTATIONS)]
    alloc_range = range(LIMIT//10, LIMIT+1, LIMIT//10)
    plot_alloc_range = []
    for i in alloc_range:
        print("\r", i, "/", LIMIT, end="")
        call_test(i)
        alloc_time = get_alloc_time()
        plot_alloc_range.append(alloc_time[0])
        for j in range(0, NUM_OF_IMPLEMENTATIONS):
            index = j * NUM_OF_IMPLEMENTATIONS
            alloc_time_plot[j].append(alloc_time[1][index])
            dealloc_time_plot[j].append(alloc_time[1][index + 1])
    print()
    plot_alloc_time(plot_alloc_range, alloc_time_plot)
    plot_dealloc_time(plot_alloc_range, dealloc_time_plot)


def plot_alloc_time(x, y):
    plt.subplot(211)
    plt.title('Allocation time of the %d implementations' %
              (NUM_OF_IMPLEMENTATIONS))
    plots = []
    for i in range(0, len(labels)):
        plots.append(plt.plot(x, y[i], label=labels[i]))
    plt.xlabel('Number of allocations')
    plt.ylabel('ms')
    plt.legend()


def plot_dealloc_time(x, y):
    plt.subplot(212)
    plt.title('Deallocation time of the %d implementations' %
              (NUM_OF_IMPLEMENTATIONS))
    plots = []
    for i in range(0, len(labels)):
        plots.append(plt.plot(x, y[i], label=labels[i]))
    plt.xlabel('Number of allocations')
    plt.ylabel('ms')
    plt.legend()


if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        try:
            EXEC = sys.argv[1]
        except:
            pass
        try:
            FILE = sys.argv[2]
        except:
            pass
        try:
            LIMIT = int(sys.argv[3])
        except:
            pass
    plot()
    plt.show()
