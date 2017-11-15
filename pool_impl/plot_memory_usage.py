#!/usr/bin/python3

import subprocess
import matplotlib.pyplot as plt


def call_massif(executable, limit):
    '''Calls the massif tool on the given executable.'''

    full_path = './build/benchmarks/memory_usage/' + executable
    subprocess.run(['valgrind', '--tool=massif', '--stacks=yes',
                    '--time-unit=ms', '--massif-out-file='+executable,
                    full_path, limit])


def parse_massif_file(file):
    snapshots = {}
    with open(file) as f:
        skip = True
        last_snapshot = 0
        for line in f:
            # we are processing a snapshot block
            if skip and line.startswith('snapshot'):
                # next lines will be interesting
                skip = False
                last_snapshot = int(line.split('=')[1])
                snapshots[last_snapshot] = {}
            # we encountered a snapshot line a few lines ago
            elif not skip:
                # skip this
                if line.startswith('#-'):
                    continue
                # end of things that we care about
                elif line.startswith('heap_tree'):
                    skip = True
                else:
                    splitted_line = line.split('=')
                    # snapshot 1 will be mapped to a dictionary
                    # which will have keys such as: mem_heap_B, etc.
                    snapshots[last_snapshot][splitted_line[0]] = \
                        int(splitted_line[1])
    return snapshots


def plot(files, format):
    formats = {'b': 1, 'k': 1024, 'm': 1048576}
    time = [[] for i in files]
    heap = [[] for i in files]
    stack = [[] for i in files]
    for idx, file in enumerate(files):
        snapshots = parse_massif_file(file)
        for _, dict2 in sorted(snapshots.items()):
            time[idx].append(dict2['time'])
            heap[idx].append((dict2['mem_heap_B'] +
                              dict2['mem_heap_extra_B']) / formats[format])
            stack[idx].append(dict2['mem_stacks_B'] / formats[format])
    formats = {'b': 'bytes', 'k': 'KiBs', 'm': 'MiBs'}
    subplot(211, 'Heap usage', time, heap, files, formats[format])
    subplot(212, 'Stack usage', time, stack, files, formats[format])


def subplot(subplot_num, title, x, y, labels, format):
    plt.subplot(subplot_num)
    plt.title(title)
    plots = []
    for i in range(0, len(labels)):
        plots.append(plt.plot(x[i], y[i], label=labels[i]))
    plt.xlabel('running time')
    plt.ylabel(format)
    plt.legend()


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Plot test data')
    parser.add_argument('--upper-bound', '-n',
                        help='The number of allocations (default: 100000)',
                        type=str, default='100000')
    parser.add_argument('--memory-format', '-m',
                        help='Can be any of <b|k|m>, '
                        'where b = bytes, k = Kibs and m = Mibs (default k)',
                        type=str, default='k')
    args = parser.parse_args()
    limit = args.upper_bound
    format = args.memory_format
    execs = ['bench_mem_normal',
             'bench_mem_linked_set',
             'bench_mem_linked_uset']
    for e in execs:
        call_massif(e, limit)
    plot(execs, format)
    plt.show()
