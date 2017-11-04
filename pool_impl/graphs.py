#!/bin/python3

import subprocess
import matplotlib.pyplot as plt


NUM_OF_IMPLEMENTATIONS = 3
labels = ["Regular", "BitPool(800)", "LinkedPool(800)"]


def get_alloc_time():
    lst = []
    with open('./time_taken.txt') as f:
        for line in f:
            if not line.endswith('.\n'):
                lst.append(float(line.split(' ')[-2]))
    return lst


def call_test(allocs, pool_size=800):
    subprocess.run(['./build/test_all', str(allocs), str(pool_size)])


def plot():
    alloc_time_plot = [[] for i in range(NUM_OF_IMPLEMENTATIONS)]
    dealloc_time_plot = [[] for i in range(NUM_OF_IMPLEMENTATIONS)]
    alloc_range = range(100000, 1000001, 100000)
    for i in alloc_range:
        print("\r", i, "/", 1000000, end="")
        call_test(i)
        alloc_time = get_alloc_time()
        for j in range(NUM_OF_IMPLEMENTATIONS):
            alloc_time_plot[j].append(alloc_time[j])
            dealloc_time_plot[j].append(alloc_time[j + NUM_OF_IMPLEMENTATIONS])
    plot_alloc_time(alloc_range, alloc_time_plot)
    plot_dealloc_time(alloc_range, dealloc_time_plot)


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
    plot()
    plt.show()
