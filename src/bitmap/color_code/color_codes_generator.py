import sys
import os
import math

if __name__ == '__main__':
    def eprint(message):
        print(message, file=sys.stderr)

    if len(sys.argv) != 3:
        eprint("Syntax: {} <colorfile> <output>".format(sys.argv[0]))
        sys.exit(1)

    codes = []
    with open(sys.argv[1]) as file:
        line_number = 0
        while True:
            line_number = line_number + 1
            line = file.readline()
            if not line:
                break
            if len(line) == 0:
                continue
            if line.startswith("##") or line[0].isspace():
                continue

            # Get the key code
            code = line[0]

            # Make sure we have more than just that
            if len(line) == 1:
                eprint("Invalid line {} on line {}".format(line, line_number))
                sys.exit(1)

            # Ignore spaces until we get something else
            line = line[1:]
            ignore_count = 0
            for c in line:
                if c.isspace():
                    ignore_count = ignore_count + 1
                    continue
                break
            line = line[ignore_count:]

            # Ignore spaces after the code
            ignore_count = 0
            for c in reversed(line):
                if c.isspace():
                    ignore_count = ignore_count + 1
                    continue
                break
            line = line[:-ignore_count]

            if len(line) < 6:
                eprint("Invalid code {} on line {}".format(line, line_number))
                sys.exit(1)

            # Convert to lowercase
            line = line.lower()

            # Get a color
            color = None

            # Check if it's xxxxxx (reset)
            if line == "xxxxxx":
                continue
            else:
                color_int = 0
                try:
                    color_int = int(line, 16)
                except ValueError:
                    eprint("Invalid code {} on line {}".format(line, line_number))
                    sys.exit(1)
                color = color_int

            # Escape the code if needed
            if code == "\\" or code == "'":
                code = "\\" + code

            # Add to the list
            codes.append([code, color])

    # Write the codes
    with open(sys.argv[2], "w") as file:
        file.write("// SPDX-License-Identifier: GPL-3.0-only\n// DON'T EDIT THIS FILE\n// Edit color_codes and then regenerate this file with the color_codes_generator.py script\n\n")
        file.write("#include <cstdint>\n")
        file.write("#include <optional>\n")
        file.write("namespace Invader {\n")
        file.write("    std::optional<std::uint32_t> color_from_color_code(std::int16_t code) {\n")
        file.write("        switch(code) {\n")
        for code in codes:
            file.write("            case \'{}\':\n".format(code[0]))
            file.write("                return 0xFF{:06X};\n".format(code[1]))
        file.write("            default: return std::nullopt;\n")
        file.write("        }\n")
        file.write("    }\n")
        file.write("}\n")
