"""File utilities."""
import json
import os
from collections import defaultdict

from common.constants import RESOURCES_DIR


def make_dirs(input_dirname, file=False):
    """Make all non-existent directories in a given file/directory relative to resources dir."""
    if file:
        dirname = os.path.join(RESOURCES_DIR, os.path.dirname(input_dirname))
    else:
        dirname = os.path.join(RESOURCES_DIR, input_dirname)
    if not os.path.exists(dirname):
        os.makedirs(dirname)
    return os.path.join(RESOURCES_DIR, input_dirname)


def unique_images(input_dirname):
    files = set()
    files_dict = defaultdict(list)
    for file in os.listdir(input_dirname):
        if file.endswith(".jpg"):
            parts = file.split("-")
            files.add(parts[1])
            files_dict[parts[1]].append(file)
    print("Number of unique files = {}".format(len(files)))
    with open(os.path.join(input_dirname, "unique_files.json"), "w") as f:
        json.dump(files_dict, f, indent=2)


if __name__ == "__main__":
    unique_images("/Users/mravuri/Desktop/evolution/images - high resolution/JPEGs")
