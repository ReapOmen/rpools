#!/usr/bin/python3

import json
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


def plot(json_file, threshold):
    """
    Plot the objects from <json_file> whose peak is higher than
    <threshold>.
    :param json_file: the file in which object allocations are recorded
    :type json_file: str
    :param threshold: the minimum peak that an object must have in order to be
                      included in the plot
    :type threshold: int
    """
    allocations = get_json(json_file)
    # index 0 is the start of the program, index n
    # means that the snapshot was taken at 100 * n ms
    x_axis = [100 * i for i in range(len(allocations))]
    # the last snapshot contains the names and peaks of all
    # the objects
    for name, v in allocations[-1].items():
        y_axis = []
        if v['peak'] >= threshold:
            for snapshot in allocations:
                y_axis.append(snapshot.get(name,
                                           {'current': 0,
                                            'peak': 0})['current'])
            plt.plot(x_axis, y_axis, label=name)
    plt.xlabel('Elapsed time (ms)')
    plt.ylabel('Number of object currently allocated')
    plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), ncol=2, loc=3)
    plt.show()


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Plot object allocation data')
    parser.add_argument('--file', '-f', help='Which file to plot')
    parser.add_argument('--threshold', '-t',
                        help="If an object's peak is smaller than the"
                        "threshold, then it will not be included in the plot"
                        ' (default: 10)', type=int, default=10)
    args = parser.parse_args()
    plot(args.file, args.threshold)
    plt.show()
