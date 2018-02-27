#!/usr/bin/python3

from jinja2 import Environment, FileSystemLoader
import json


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


if __name__ == "__main__":
    import argparse
    import os
    parser = argparse.ArgumentParser(description='Plot object allocation data')
    parser.add_argument('--file', '-f', help='Which file to plot')
    args = parser.parse_args()
    snapshots = get_json(args.file)
    j2_env = Environment(loader=FileSystemLoader('./debug'),
                         trim_blocks=True)
    headers = [('Snapshot', 'number'), ('Type Name', ''),
               ('Alignment', 'number'), ('Size of allocation', 'number'),
               ('Currently allocated', 'number'), ('Peak', 'number'),
               ('Allocated in', '')]
    num_of_snapshots = len(snapshots)
    num_of_snapshots -= 1 if snapshots[0] is None else 0
    types = [name for name in snapshots[-1]]
    functions = set()
    for type, aobj in snapshots[-1].items():
        for align, sizes in aobj.items():
            for size, obj in sizes.items():
                functions.add(obj['function'])
    out = (j2_env.get_template('base.html').render(
        headers=headers,
        snapshots=snapshots,
        num_of_snapshots=num_of_snapshots,
        types=types,
        functions=functions))
    html_file = os.path.splitext(os.path.basename(args.file))[0]
    with open(html_file + '.html', 'wb') as fh:
        fh.write(out.encode('utf-8'))
