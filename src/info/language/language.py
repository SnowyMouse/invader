import sys
import json
import os

if len(sys.argv) < 3:
    printf("Usage: {} <output.cpp> <a.json> [b.json] [...]")
    sys.exit(1)

READER_VERSION = 1

languages = {}

for i in sys.argv[2:]:
    with open(i) as f:
        d = json.load(f)
        if d["version"] != READER_VERSION:
            printf("Version ({}) does not match reader ({})".format(d["version"], READER_VERSION))
            sys.exit(1)
        languages[os.path.basename(i).split(".")[0]] = d

with open(sys.argv[1], "w") as f:
    f.write("// This value was auto-generated. Changes made to this file may get overwritten.\n")
    f.write("#include <string>\n")
    f.write("#include <vector>\n")
    f.write("namespace Invader {\n")
    for q in languages:
        def write_offsets(what):
            for k in what:
                f.write("        {{0x{:08X},0x{:08X}}},\n".format(k["offset"], k["size"]))
        f.write("    static std::size_t {}_bitmaps[][2] = {{\n".format(q))
        write_offsets(languages[q]["bitmaps"])
        f.write("    };\n")
        f.write("    static std::size_t {}_sounds[][2] = {{\n".format(q))
        write_offsets(languages[q]["sounds"])
        f.write("    };\n")
    f.write("    std::vector<std::string> get_languages_for_resources(const std::size_t *bitmaps_offsets, const std::size_t *bitmaps_sizes, std::size_t bitmap_count, const std::size_t *sounds_offsets, const std::size_t *sounds_sizes, std::size_t sounds_count, bool &universal) {\n")
    f.write("        std::vector<std::string> languages;\n")
    f.write("        universal = true;\n")
    for q in languages:
        f.write("        languages.emplace_back(\"{}\");\n".format(q))
    f.write("        auto remove_language = [&languages, &universal](const char *language) {\n")
    f.write("            for(auto &l : languages) {\n")
    f.write("                if(l == language) {\n")
    f.write("                    languages.erase(languages.begin() + (&l - languages.data()));\n")
    f.write("                    universal = false;\n")
    f.write("                    return;\n")
    f.write("                }\n")
    f.write("            }\n")
    f.write("        };\n\n")
    f.write("        for(std::size_t b = 0; b < bitmap_count; b++) {\n")
    f.write("            bool found = false;\n")
    for q in languages:
        f.write("            for(auto &l : {}_bitmaps) {{\n".format(q))
        f.write("                if(l[0] == bitmaps_offsets[b]) {\n")
        f.write("                    found = l[1] == bitmaps_sizes[b];\n")
        f.write("                    break;\n")
        f.write("                }\n")
        f.write("            }\n")
        f.write("            if(!found) {\n")
        f.write("                remove_language(\"{}\");\n".format(q))
        f.write("            }\n")
        f.write("            found = false;\n")
    f.write("        }\n\n")
    f.write("        for(std::size_t s = 0; s < sounds_count; s++) {\n")
    f.write("            bool found = false;\n")
    for q in languages:
        f.write("            for(auto &l : {}_sounds) {{\n".format(q))
        f.write("                if(l[0] == sounds_offsets[s]) {\n")
        f.write("                    found = l[1] == sounds_sizes[s];\n")
        f.write("                    break;\n")
        f.write("                }\n")
        f.write("            }\n")
        f.write("            if(!found) {\n")
        f.write("                remove_language(\"{}\");\n".format(q))
        f.write("            }\n")
        f.write("            found = false;\n")
    f.write("        }\n")
    f.write("        return languages;\n".format(i))
    f.write("    }\n")
    f.write("}\n")
