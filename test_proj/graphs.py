#!/bin/python3

import subprocess
import matplotlib.pyplot as plt


def get_overheads():
    lst = []
    with open('./overheads.txt') as f:
        for line in f:
            lst.append(int(line.split(' ')[-1]))
    return lst


def get_alloc_time():
    lst = []
    with open('./time_taken.txt') as f:
        for line in f:
            if not line.endswith('.\n'):
                lst.append(float(line.split(' ')[-2]))
    return lst


def call_test(allocs, pool_size=100):
    subprocess.run(['./build/test', str(allocs), str(pool_size)])


def plot():
    overheads_to_plot = [[], [], []]
    alloc_time_plot = [[], [], []]
    dealloc_time_plot = [[], [], []]
    alloc_range = range(1, 10000, 100)
    for i in alloc_range:
        call_test(i)
        overheads = get_overheads()
        overheads_to_plot[0].append(overheads[0])
        overheads_to_plot[1].append(overheads[1])
        overheads_to_plot[2].append(overheads[2])
        alloc_time = get_alloc_time()
        alloc_time_plot[0].append(alloc_time[0])
        alloc_time_plot[1].append(alloc_time[1])
        alloc_time_plot[2].append(alloc_time[2])
        dealloc_time_plot[0].append(alloc_time[3])
        dealloc_time_plot[1].append(alloc_time[4])
        dealloc_time_plot[2].append(alloc_time[5])
    plot_overheads(alloc_range, overheads_to_plot)
    plot_alloc_time(alloc_range, alloc_time_plot)
    plot_dealloc_time(alloc_range, dealloc_time_plot)


def plot_overheads(x, y):
    plt.subplot(221)
    plt.title('Memory overheads of the 3 implementations')
    labels = ["SomeObject", "SomeObject2", "SomeObject3(100)"]
    plots = []
    for i in range(0, len(labels)):
        plots.append(plt.plot(x, y[i], label=labels[i]))
    plt.xlabel('Number of allocations')
    plt.ylabel('Bytes')
    plt.legend()


def plot_alloc_time(x, y):
    plt.subplot(222)
    plt.title('Allocation time of the 3 implementations')
    labels = ["SomeObject", "SomeObject2", "SomeObject3(100)"]
    plots = []
    for i in range(0, len(labels)):
        plots.append(plt.plot(x, y[i], label=labels[i]))
    plt.xlabel('Number of allocations')
    plt.ylabel('ms')
    plt.legend()


def plot_dealloc_time(x, y):
    plt.subplot(224)
    plt.title('Deallocation time of the 3 implementations')
    labels = ["SomeObject", "SomeObject2", "SomeObject3(100)"]
    plots = []
    for i in range(0, len(labels)):
        plots.append(plt.plot(x, y[i], label=labels[i]))
    plt.xlabel('Number of allocations')
    plt.ylabel('ms')
    plt.legend()


if __name__ == "__main__":
    plot()
    plt.show()
