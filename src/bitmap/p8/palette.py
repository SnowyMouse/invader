import sys
import math

if len(sys.argv) != 3:
    print("Usage: {} <input> <output>".format(sys.argv[0]), file=sys.stderr)
    sys.exit(1)

colors = []
color_data = []

palette_indices = []

# Open and read it
with open(sys.argv[1], "rb") as palette_file:
    palette_file.seek(0,2)
    palette_file_size = palette_file.tell()
    if palette_file_size != 256 * 4:
        print("Error: Palette has an invalid size. Expected {} bytes but got {}".format(256 * 4, palette_file_size), file=sys.stderr)
        sys.exit(1)
    palette_file.seek(0,0)
    color_data = palette_file.read(256 * 4)

# Make the colors array
colors = [(int(color_data[i * 4]), int(color_data[i * 4 + 1]), int(color_data[i * 4 + 2]), int(color_data[i * 4 + 3])) for i in range(0,256)]

# Find the first black
first_black = 0
for c in range(0,256):
    if colors[c][0] == 0:
        first_black = c - 1
        break

# Go through each red and green. Since it's normalized, blue can be inferred
for red in range(0,256):
    for green in range(0,256):
        closest_error = 65536
        closest_index = None
        for color_index in range(0,first_black):
            color = colors[color_index]
            distance_x = (color[1] - red)
            distance_y = (color[2] - green)
            distance = distance_x * distance_x + distance_y * distance_y

            if distance < closest_error:
                closest_error = distance
                closest_index = color_index

        palette_indices.append(closest_index)

# Now write the C++ file
with open(sys.argv[2], "w") as cpp:
    cpp.write("// This value was auto-generated. Changes made to this file may get overwritten.\n")
    cpp.write("#include <cstdint>\n")
    cpp.write("namespace Invader {\n")

    cpp.write("    static constexpr std::uint8_t p8_map[] = {")
    for color in range(0,len(palette_indices)):
        if color % 16 == 0:
            cpp.write("\n        ")
        cpp.write("0x{:02X}{}".format(palette_indices[color], "," if color + 1 < len(palette_indices) else "\n"))
    cpp.write("    };\n")

    cpp.write("    static constexpr std::uint8_t p8_colors[256][4] = {")
    for color in range(0,len(colors)):
        if color % 16 == 0:
            cpp.write("\n        ")
        cpp.write("0x{:02X},0x{:02X},0x{:02X},0x{:02X}{}".format(colors[color][0], colors[color][1], colors[color][2], colors[color][3], "," if color + 1 < len(colors) else "\n"))
    cpp.write("    };\n")

    cpp.write("    std::uint8_t rg_convert_to_p8(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {\n")
    cpp.write("        if(alpha < 0x2F) {\n")
    cpp.write("            // Basically add 1 if blue is at least 0x80.\n")
    cpp.write("            return 0xFE + (blue >= 0x80);\n")
    cpp.write("        }\n")
    cpp.write("        return p8_map[red * 256 + green];\n")
    cpp.write("    }\n")

    cpp.write("    void p8_convert_to_rgba(std::uint8_t p8, std::uint8_t &red, std::uint8_t &green, std::uint8_t &blue, std::uint8_t &alpha) {\n")
    cpp.write("        alpha = p8_colors[p8][0];\n")
    cpp.write("        red = p8_colors[p8][1];\n")
    cpp.write("        green = p8_colors[p8][2];\n")
    cpp.write("        blue = p8_colors[p8][3];\n")
    cpp.write("    }\n")



    cpp.write("}\n")
