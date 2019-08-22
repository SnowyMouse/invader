#!/bin/python3
# Invader (c) 2019 Kavawuvi
#
# This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
#

import sys

if len(sys.argv) != 5:
    print("Usage: {} <bitmaps.tag_indices> <sounds.tag_indices> <loc.tag_indices> <output.cpp>".format(sys.argv[0]))
    exit(1)

def read_indices(fname):
    with open(fname, "r") as f:
        # Get the first line, the count
        count = int(f.readline())

        # Get the rest
        lines = [line for line in f]

        # Verify that it is, indeed, an index file
        if count != len(lines):
            print("Invalid index file {} ({} count != {} tags on file)".format(fname, count, len(lines)))
            exit(1)

        # Return what we got
        return lines

bitmaps = read_indices(sys.argv[1])
sounds = read_indices(sys.argv[2])
loc = read_indices(sys.argv[3])

with open(sys.argv[4], "w") as f:
    f.write("// This value was auto-generated. Changes made to this file may get overwritten.\n")
    f.write("namespace Invader {\n")
    def write_function(fname, list):
        f.write("    const char **{}() {{\n".format(fname))
        f.write("        static const char *array[] = {\n")
        for i in list:
            f.write("            \"{}\",\n".format(i.rstrip("\n").replace("\\", "\\\\")))
        f.write("            nullptr\n")
        f.write("        };\n")
        f.write("        return array;\n")
        f.write("    }\n")
        f.write("    unsigned long {}_count() {{\n".format(fname))
        f.write("        return {};\n".format(len(list)))
        f.write("    }\n")

    write_function("get_default_bitmap_resources", bitmaps)
    write_function("get_default_sound_resources", sounds)
    write_function("get_default_loc_resources", loc)
    f.write("}\n")
