# SPDX-License-Identifier: GPL-3.0-only

import sys
import json
import os

if len(sys.argv) < 11:
    print("Usage: {} <definition.hpp> <parser.hpp> <parser-save-hek-data.cpp> <parser-read-hek-data.cpp> <parser-read-cache-file-data.cpp> <parser-cache-format.cpp> <parser-cache-deformat.cpp> <enum.cpp> <extract-hidden> <json> [json [...]]".format(sys.argv[0]), file=sys.stderr)

files = []
all_enums = []
all_bitfields = []
all_structs = []

extract_hidden = True if sys.argv[8].lower() == "on" else False

for i in range(10, len(sys.argv)):
    def make_name_fun(name, ignore_numbers):
        name = name.replace(" ", "_").replace("'", "").replace("-","_")
        if not ignore_numbers and name[0].isnumeric():
            name = "_{}".format(name)
        return name

    objects = None
    with open(sys.argv[i], "r") as f:
        objects = json.loads(f.read())
    name = os.path.basename(sys.argv[i]).split(".")[0]
    files.append(name)

    # Get all enums, bitfields, and structs
    for s in objects:
        if s["type"] == "enum":
            for o in range(len(s["options"])):
                s["options"][o] = make_name_fun(s["options"][o], True)
            all_enums.append(s)
        elif s["type"] == "bitfield":
            for f in range(len(s["fields"])):
                s["fields"][f] = make_name_fun(s["fields"][f], False)
            all_bitfields.append(s)
        elif s["type"] == "struct":
            for f in s["fields"]:
                if f["type"] != "pad":
                    f["name"] = make_name_fun(f["name"], False)
            all_structs.append(s)
        else:
            print("Unknown object type {}".format(s["type"]), file=sys.stderr)
            sys.exit(1)

# Basically, we're going to be rearranging structs so that structs that don't have dependencies get added first. Structs that do get added last, and they only get added once their dependencies are added
all_structs_arranged = []
def add_struct(name):
    # Make sure we aren't already added
    for s in all_structs_arranged:
        if s["name"] == name:
            return

    # Get all their dependencies
    dependencies = []

    # Get the struct
    struct_to_add = None
    for s in all_structs:
        if s["name"] == name:
            struct_to_add = s
            break

    if not struct_to_add:
        if name != "PredictedResource":
            print("Warning: Unknown struct {}".format(name), file=sys.stderr)
        return

    if "inherits" in struct_to_add:
        dependencies.append(struct_to_add["inherits"])

    for f in s["fields"]:
        if f["type"] == "TagReflexive":
            if f["struct"] not in dependencies:
                dependencies.append(f["struct"])

    for d in dependencies:
        add_struct(d)

    all_structs_arranged.append(struct_to_add)

for s in all_structs:
    add_struct(s["name"])

def to_hex(number):
    return "0x{:X}".format(number)

with open(sys.argv[1], "w") as f:
    f.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    header_name = "INVADER__TAG__HEK__CLASS__DEFINITION_HPP"
    f.write("#ifndef {}\n".format(header_name))
    f.write("#define {}\n\n".format(header_name))
    f.write("#include \"../../hek/data_type.hpp\"\n\n")
    f.write("namespace Invader::HEK {\n")

    ecpp = open(sys.argv[8], "w")
    ecpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    ecpp.write("#include <cstring>\n")
    ecpp.write("#include <cctype>\n")
    ecpp.write("#include <invader/tag/hek/definition.hpp>\n\n")
    ecpp.write("namespace Invader::HEK {\n")
    ecpp.write("    static inline bool equal_string_case_insensitive(const char *a, const char *b) noexcept {\n")
    ecpp.write("        while(std::tolower(*a) == std::tolower(*b)) {\n")
    ecpp.write("            if(*a == 0) {\n")
    ecpp.write("                return true;\n")
    ecpp.write("            }\n")
    ecpp.write("            a++; b++;\n")
    ecpp.write("        }\n")
    ecpp.write("        return false;\n")
    ecpp.write("    }\n")

    # Write enums at the top first, then bitfields
    for e in all_enums:
        f.write("    enum {} : TagEnum {{\n".format(e["name"]))

        # Convert PascalCase to UPPER_SNAKE_CASE
        prefix = ""
        name_to_consider = e["name"].replace("HUD", "Hud").replace("UI", "Ui")
        for i in name_to_consider:
            if prefix != "" and i.isupper():
                prefix += "_"
            prefix += i.upper()

        def format_enum(value):
            return "{}_{}".format(prefix,value.upper())

        def format_enum_str(value):
            return value.replace("_", "-")

        for n in range(0,len(e["options"])):
            f.write("        {}{}\n".format(format_enum(e["options"][n]), "," if n + 1 < len(e["options"]) else ""))

        f.write("    };\n")

        f.write("    /**\n")
        f.write("     * Get the string representation of the enum.\n")
        f.write("     * @param value value of the enum\n")
        f.write("     * @return      string representation of the enum\n")
        f.write("     */\n")
        f.write("    const char *{}_to_string({} value);\n".format(e["name"], e["name"]))
        ecpp.write("    const char *{}_to_string({} value) {{\n".format(e["name"], e["name"]))
        ecpp.write("        switch(value) {\n")
        for n in e["options"]:
            ecpp.write("        case {}::{}:\n".format(e["name"], format_enum(n)))
            ecpp.write("            return \"{}\";\n".format(format_enum_str(n)))
        ecpp.write("        default:\n")
        ecpp.write("            throw std::exception();\n")
        ecpp.write("        }\n")
        ecpp.write("    }\n")

        f.write("    /**\n")
        f.write("     * Get the enum value from the string.\n")
        f.write("     * @param value value of the enum as a string\n")
        f.write("     * @return      value of the enum\n")
        f.write("     */\n")
        f.write("    {} {}_from_string(const char *value);\n".format(e["name"], e["name"]))
        ecpp.write("    {} {}_from_string(const char *value) {{\n".format(e["name"], e["name"]))
        for n in range(0,len(e["options"])):
            ecpp.write("        {}if(equal_string_case_insensitive(value, \"{}\")) {{\n".format("" if n == 0 else "else ", format_enum_str(e["options"][n])))
            ecpp.write("             return {}::{};\n".format(e["name"], format_enum(e["options"][n])))
            ecpp.write("        }\n")
        ecpp.write("        else {\n")
        ecpp.write("            throw std::exception();\n")
        ecpp.write("        }\n")
        ecpp.write("    }\n")

    for b in all_bitfields:
        f.write("    struct {} {{\n".format(b["name"]))
        for q in b["fields"]:
            f.write("        std::uint{}_t {} : 1;\n".format(b["width"], q))
        f.write("    };\n")

    ecpp.write("}\n")
    ecpp.close()

    # Now the hard part
    padding_present = False

    for s in all_structs_arranged:
        f.write("    ENDIAN_TEMPLATE(EndianType) struct {} {}{{\n".format(s["name"], ": {}<EndianType> ".format(s["inherits"]) if "inherits" in s else ""))
        for n in s["fields"]:
            type_to_write = n["type"]

            if type_to_write.startswith("int") or type_to_write.startswith("uint"):
                type_to_write = "std::{}_t".format(type_to_write)
            elif type_to_write == "pad":
                f.write("        PAD(0x{:X});\n".format(n["size"]))
                continue

            if "flagged" in n and n["flagged"]:
                type_to_write = "FlaggedInt<{}>".format(type_to_write)

            name = n["name"]
            if "count" in n:
                name = "{}[{}]".format(name, n["count"])

            format_to_use = None

            default_endian = "EndianType"
            if "endian" in n:
                if n["endian"] == "little":
                    default_endian = "LittleEndian"
                elif n["endian"] == "big":
                    default_endian = "BigEndian"
                elif n["endian"] == None:
                    default_endian = None

            if type_to_write == "TagReflexive":
                f.write("        TagReflexive<{}, {}> {};\n".format(default_endian, n["struct"], name))
            else:
                if default_endian is None:
                    format_to_use = "{}"
                elif "compound" in n and n["compound"]:
                    format_to_use = "{{}}<{}>".format(default_endian)
                else:
                    format_to_use = "{}<{{}}>".format(default_endian)

                if "bounds" in n and n["bounds"]:
                    f.write("        Bounds<{}> {};\n".format(format_to_use.format(type_to_write), name))
                else:
                    f.write("        {} {};\n".format(format_to_use.format(type_to_write), name))

        # Make sure we have all of the structs we depend on, too
        depended_structs = []
        dependency = s
        padding_present = False

        while dependency is not None:
            depended_structs.append(dependency)
            if not padding_present:
                for n in dependency["fields"]:
                    if n["type"] == "pad":
                        padding_present = True
                        break
            if "inherits" in dependency:
                dependency_name = dependency["inherits"]
                dependency = None
                for ds in all_structs_arranged:
                    if ds["name"] == dependency_name:
                        dependency = ds
                        break
            else:
                break

        # And we can't forget the copy part
        f.write("        ENDIAN_TEMPLATE(NewEndian) operator {}<NewEndian>() const {{\n".format(s["name"]))
        f.write("            {}<NewEndian> copy{};\n".format(s["name"], " = {}" if padding_present else ""))

        for ds in depended_structs:
            for n in ds["fields"]:
                if n["type"] == "pad":
                    continue
                else:
                    f.write("            {}({});\n".format("COPY_THIS_ARRAY" if "count" in n else "COPY_THIS", n["name"]))
        f.write("            return copy;\n")
        f.write("        }\n")

        f.write("    };\n")
        f.write("    static_assert(sizeof({}<NativeEndian>) == 0x{:X});\n".format(s["name"], s["size"]))

    f.write("}\n\n")
    f.write("#endif\n")

hpp = open(sys.argv[2], "w")
cpp_save_hek_data = open(sys.argv[3], "w")
cpp_read_cache_file_data = open(sys.argv[4], "w")
cpp_read_hek_data = open(sys.argv[5], "w")
cpp_cache_format_data = open(sys.argv[6], "w")
cpp_cache_deformat_data = open(sys.argv[7], "w")

def write_for_all_cpps(what):
    cpp_save_hek_data.write(what)
    cpp_read_cache_file_data.write(what)
    cpp_read_hek_data.write(what)
    cpp_cache_format_data.write(what)
    cpp_cache_deformat_data.write(what)

hpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
write_for_all_cpps("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
header_name = "INVADER__TAG__PARSER__PARSER_HPP"
hpp.write("#ifndef {}\n".format(header_name))
hpp.write("#define {}\n\n".format(header_name))
hpp.write("#include <string>\n")
hpp.write("#include <optional>\n")
hpp.write("#include \"../../map/map.hpp\"\n")
hpp.write("#include \"../hek/definition.hpp\"\n\n")
hpp.write("namespace Invader {\n")
hpp.write("    class BuildWorkload;\n")
hpp.write("}\n")
hpp.write("namespace Invader::Parser {\n")
hpp.write("    struct Dependency {\n")
hpp.write("        TagClassInt tag_class_int;\n")
hpp.write("        std::string path;\n")
hpp.write("        HEK::TagID tag_id = HEK::TagID::null_tag_id();\n")
hpp.write("    };\n")

write_for_all_cpps("#include <invader/tag/parser/parser.hpp>\n")
write_for_all_cpps("#include <invader/map/map.hpp>\n")
write_for_all_cpps("#include <invader/map/tag.hpp>\n")
write_for_all_cpps("#include <invader/tag/hek/header.hpp>\n")
write_for_all_cpps("#include <invader/printf.hpp>\n")
cpp_cache_format_data.write("#include <invader/build/build_workload.hpp>\n")
cpp_read_cache_file_data.write("#include <invader/file/file.hpp>\n")
cpp_save_hek_data.write("extern \"C\" std::uint32_t crc32(std::uint32_t crc, const void *buf, std::size_t size) noexcept;\n")
write_for_all_cpps("namespace Invader::Parser {\n")

for s in all_structs_arranged:
    struct_name = s["name"]
    post_cache_deformat = "post_cache_deformat" in s and s["post_cache_deformat"]
    post_cache_parse = "post_cache_parse" in s and s["post_cache_parse"]
    pre_compile = "pre_compile" in s and s["pre_compile"]
    post_compile = "post_compile" in s and s["post_compile"]
    private_functions = post_cache_deformat

    hpp.write("    struct {} {{\n".format(struct_name))
    hpp.write("        using struct_big = HEK::{}<HEK::BigEndian>;\n".format(struct_name))
    hpp.write("        using struct_little = HEK::{}<HEK::LittleEndian>;\n".format(struct_name))
    all_used_structs = []
    def add_structs_from_struct(struct):
        if "inherits" in struct:
            for t in all_structs:
                if t["name"] == struct["inherits"]:
                    add_structs_from_struct(t)
                    break
        for t in struct["fields"]:
            if t["type"] == "pad":
                continue
            type_to_write = t["type"]
            non_type = False
            if type_to_write.startswith("int") or type_to_write.startswith("uint"):
                type_to_write = "std::{}_t".format(type_to_write)
                non_type = True
            elif type_to_write == "float":
                type_to_write = "float"
                non_type = True
            elif type_to_write == "TagDependency":
                type_to_write = "Dependency"
                non_type = True
            elif type_to_write == "TagReflexive":
                if t["struct"] == "PredictedResource":
                    continue
                type_to_write = "std::vector<{}>".format(t["struct"])
                non_type = True
            elif type_to_write == "TagDataOffset":
                type_to_write = "std::vector<std::byte>"
                non_type = True
            else:
                type_to_write = "HEK::{}".format(type_to_write)
            if "flagged" in t and t["flagged"]:
                type_to_write = "HEK::FlaggedInt<{}>".format(type_to_write)
            if "compound" in t and t["compound"] and not non_type:
                type_to_write = "{}<HEK::NativeEndian>".format(type_to_write)
            if "bounds" in t and t["bounds"]:
                type_to_write = "HEK::Bounds<{}>".format(type_to_write)
            hpp.write("        {} {}{};\n".format(type_to_write, t["name"], "" if "count" not in t or t["count"] == 1 else "[{}]".format(t["count"])))
            all_used_structs.append(t)
            continue
    add_structs_from_struct(s)

    hpp.write("\n        /**\n")
    hpp.write("         * Get whether or not the data is formatted for cache files.\n")
    hpp.write("         * @return true if data is formatted for cache files\n")
    hpp.write("         */\n")
    hpp.write("        bool is_cache_formatted() const noexcept;\n")
    cpp_cache_deformat_data.write("    bool {}::is_cache_formatted() const noexcept {{\n".format(struct_name))
    cpp_cache_deformat_data.write("        return this->cache_formatted;\n")
    cpp_cache_deformat_data.write("    }\n")

    hpp.write("\n        /**\n")
    hpp.write("         * Format the tag to be used in HEK tags.\n")
    hpp.write("         */\n")
    hpp.write("        void cache_deformat();\n")
    cpp_cache_deformat_data.write("    void {}::cache_deformat() {{\n".format(struct_name))
    cpp_cache_deformat_data.write("        if(this->cache_formatted) {\n")
    for struct in all_used_structs:
        if struct["type"] == "TagReflexive":
            cpp_cache_deformat_data.write("            for(auto &i : {}) {{\n".format(struct["name"]))
            cpp_cache_deformat_data.write("                i.cache_deformat();\n")
            cpp_cache_deformat_data.write("            }\n")
    cpp_cache_deformat_data.write("            this->cache_formatted = false;\n")
    if post_cache_deformat:
        cpp_cache_deformat_data.write("            this->post_cache_deformat();\n")
    cpp_cache_deformat_data.write("        }\n")
    cpp_cache_deformat_data.write("    }\n")

    # compile()
    hpp.write("\n        /**\n")
    hpp.write("         * Compile the tag to be used in cache files.\n")
    hpp.write("         * @param workload     workload struct to use\n")
    hpp.write("         * @param tag_index    tag index to use in the workload\n")
    hpp.write("         * @param struct_index struct index to use in the workload\n")
    hpp.write("         * @param bsp          BSP index to use\n")
    hpp.write("         * @param offset       struct offset\n")
    hpp.write("         */\n")
    hpp.write("        void compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp = std::nullopt, std::size_t offset = 0);\n")
    cpp_cache_format_data.write("    void {}::compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp, std::size_t offset) {{\n".format(struct_name))
    cpp_cache_format_data.write("        auto *start = workload.structs[struct_index].data.data();\n")
    cpp_cache_format_data.write("        workload.structs[struct_index].bsp = bsp;\n")
    cpp_cache_format_data.write("        workload.structs[struct_index].unsafe_to_dedupe = {};\n".format("true" if ("unsafe_to_dedupe" in s and s["unsafe_to_dedupe"]) else "false"))
    if pre_compile:
        cpp_cache_format_data.write("        if(!this->cache_formatted) {\n")
        cpp_cache_format_data.write("            this->pre_compile(workload, tag_index, struct_index, offset);\n")
        cpp_cache_format_data.write("        }\n")
        cpp_cache_format_data.write("        this->cache_formatted = true;\n")
    cpp_cache_format_data.write("        auto &r = *reinterpret_cast<struct_little *>(start + offset + tag_index * 0);\n")
    cpp_cache_format_data.write("        std::fill(reinterpret_cast<std::byte *>(&r), reinterpret_cast<std::byte *>(&r), std::byte());\n")
    for struct in all_used_structs:
        if ("non_cached" in struct and struct["non_cached"]) or ("compile_ignore" in struct and struct["compile_ignore"]):
            continue
        name = struct["name"]
        if struct["type"] == "TagDependency":
            cpp_cache_format_data.write("        this->{}.tag_id = HEK::TagID::null_tag_id();\n".format(name))
            cpp_cache_format_data.write("        r.{}.tag_class_int = this->{}.tag_class_int;\n".format(name, name))
            cpp_cache_format_data.write("        if(this->{}.path.size() > 0) {{\n".format(name))
            cpp_cache_format_data.write("            std::size_t index = workload.compile_tag_recursively(this->{}.path.data(), this->{}.tag_class_int);\n".format(name, name))
            cpp_cache_format_data.write("            this->{}.tag_id.index = static_cast<std::uint16_t>(index);\n".format(name))
            cpp_cache_format_data.write("            auto &d = workload.structs[struct_index].dependencies.emplace_back();\n")
            cpp_cache_format_data.write("            d.offset = reinterpret_cast<std::byte *>(&r.{}) - start;\n".format(name))
            cpp_cache_format_data.write("            d.tag_index = index;\n".format(name))
            cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        else {\n")
            if "non_null" in struct and struct["non_null"]:
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{} must not be null\", tag_index);\n".format(name))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
            else:
                cpp_cache_format_data.write("            r.{}.tag_id = HEK::TagID::null_tag_id();\n".format(name))
            cpp_cache_format_data.write("        }\n")
        elif struct["type"] == "TagReflexive":
            cpp_cache_format_data.write("        std::size_t t_{}_count = this->{}.size();\n".format(name, name))
            if "minimum" in struct:
                minimum = struct["minimum"]
                cpp_cache_format_data.write("        if(t_{}_count < {}) {{\n".format(name, minimum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{} must have at least {} block{}\", tag_index);\n".format(name, minimum, "" if minimum == 1 else "s"))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            if "maximum" in struct:
                maximum = struct["maximum"]
                cpp_cache_format_data.write("        if(t_{}_count > {}) {{\n".format(name, maximum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{} must have no more than {} block{}\", tag_index);\n".format(name, maximum, "" if maximum == 1 else "s"))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        if(t_{}_count > 0) {{\n".format(name))
            cpp_cache_format_data.write("            r.{}.count = static_cast<std::uint32_t>(t_{}_count);\n".format(name, name))
            cpp_cache_format_data.write("            auto &n = workload.structs.emplace_back();\n")
            cpp_cache_format_data.write("            static constexpr std::size_t STRUCT_SIZE = sizeof({}::struct_little);\n".format(struct["struct"]))
            cpp_cache_format_data.write("            n.data.resize(t_{}_count * STRUCT_SIZE);\n".format(name))
            cpp_cache_format_data.write("            auto &p = workload.structs[struct_index].pointers.emplace_back();\n")
            cpp_cache_format_data.write("            p.struct_index = &n - workload.structs.data();\n")
            cpp_cache_format_data.write("            p.offset = reinterpret_cast<std::byte *>(&r.{}.pointer) - start;\n".format(name))
            cpp_cache_format_data.write("            for(std::size_t i = 0; i < t_{}_count; i++) {{\n".format(name))
            cpp_cache_format_data.write("                try {\n")
            cpp_cache_format_data.write("                    this->{}[i].compile(workload, tag_index, p.struct_index, bsp, i * STRUCT_SIZE);\n".format(name))
            cpp_cache_format_data.write("                }\n")
            cpp_cache_format_data.write("                catch(std::exception &) {\n")
            cpp_cache_format_data.write("                    eprintf(\"Failed to compile {}::{} #%zu\\n\", i);\n".format(struct_name, name))
            cpp_cache_format_data.write("                    throw;\n")
            cpp_cache_format_data.write("                }\n")
            cpp_cache_format_data.write("            }\n")
            cpp_cache_format_data.write("        }\n")
        elif struct["type"] == "TagDataOffset":
            cpp_cache_format_data.write("        std::size_t t_{}_size = this->{}.size();\n".format(name, name))
            cpp_cache_format_data.write("        if(t_{}_size > 0) {{\n".format(name))
            cpp_cache_format_data.write("            auto &n = workload.structs.emplace_back();\n")
            cpp_cache_format_data.write("            n.bsp = bsp;\n")
            cpp_cache_format_data.write("            n.data.insert(n.data.begin(), this->{}.begin(), this->{}.end());\n".format(name, name))
            cpp_cache_format_data.write("            auto &p = workload.structs[struct_index].pointers.emplace_back();\n")
            cpp_cache_format_data.write("            p.struct_index = &n - workload.structs.data();\n")
            cpp_cache_format_data.write("            p.offset = reinterpret_cast<std::byte *>(&r.{}.pointer) - start;\n".format(name))
            cpp_cache_format_data.write("            r.{}.size = t_{}_size;\n".format(name, name))
            cpp_cache_format_data.write("        }\n")
        elif "bounds" in struct and struct["bounds"]:
            cpp_cache_format_data.write("        r.{}.from = this->{}.from;\n".format(name, name))
            cpp_cache_format_data.write("        r.{}.to = this->{}.to;\n".format(name, name))
        elif "count" in struct and struct["count"] > 1:
            cpp_cache_format_data.write("        std::copy(this->{}, this->{} + {}, r.{});\n".format(name, name, struct["count"], name))
        elif struct["type"] == "enum":
            cpp_cache_format_data.write("        if(static_cast<std::uint16_t>(r.{}) >= {}) {{\n".format(name, len(struct["options"])))
            cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{} exceeds maximum value of {}\", tag_index);\n".format(name, len(struct["options"])))
            cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
            cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        r.{} = this->{};\n".format(name, name))
        else:
            cpp_cache_format_data.write("        r.{} = this->{};\n".format(name, name))
    if post_compile:
        cpp_cache_format_data.write("        this->post_compile(workload, tag_index, struct_index, offset);\n".format(name, name))
    cpp_cache_format_data.write("    }\n")

    # generate_hek_tag_data()
    hpp.write("\n        /**\n")
    hpp.write("         * Convert the struct into HEK tag data to be built into a cache file.\n")
    hpp.write("         * @param  generate_header_class generate a cache file header with the class, too\n")
    hpp.write("         * @param  clear_on_save         clear data as it's being saved (reduces memory usage but you can't work on the tag anymore)\n")
    hpp.write("         * @return cache file data\n")
    hpp.write("         */\n")
    hpp.write("        std::vector<std::byte> generate_hek_tag_data(std::optional<TagClassInt> generate_header_class = std::nullopt, bool clear_on_save = false);\n")
    cpp_save_hek_data.write("    std::vector<std::byte> {}::generate_hek_tag_data(std::optional<TagClassInt> generate_header_class, bool clear_on_save) {{\n".format(struct_name))
    cpp_save_hek_data.write("        this->cache_deformat();\n")
    cpp_save_hek_data.write("        std::vector<std::byte> converted_data(sizeof(struct_big));\n")
    cpp_save_hek_data.write("        std::size_t tag_header_offset = 0;\n")
    cpp_save_hek_data.write("        if(generate_header_class.has_value()) {\n")
    cpp_save_hek_data.write("            HEK::TagFileHeader header(*generate_header_class);\n")
    cpp_save_hek_data.write("            tag_header_offset = sizeof(header);\n")
    cpp_save_hek_data.write("            converted_data.insert(converted_data.begin(), reinterpret_cast<std::byte *>(&header), reinterpret_cast<std::byte *>(&header + 1));\n")
    cpp_save_hek_data.write("        }\n")
    if len(all_used_structs) > 0:
        cpp_save_hek_data.write("        struct_big b = {};\n")
        for struct in all_used_structs:
            if ("cache_only" in struct and struct["cache_only"] and not extract_hidden):
                continue
            name = struct["name"]
            if struct["type"] == "TagDependency":
                cpp_save_hek_data.write("        std::size_t {}_size = static_cast<std::uint32_t>(this->{}.path.size());\n".format(name,name))
                cpp_save_hek_data.write("        b.{}.tag_class_int = this->{}.tag_class_int;\n".format(name, name))
                cpp_save_hek_data.write("        b.{}.tag_id = HEK::TagID::null_tag_id();\n".format(name))
                cpp_save_hek_data.write("        if({}_size > 0) {{\n".format(name))
                cpp_save_hek_data.write("            b.{}.path_size = static_cast<std::uint32_t>({}_size);\n".format(name, name))
                cpp_save_hek_data.write("            converted_data.insert(converted_data.end(), reinterpret_cast<std::byte *>(this->{}.path.data()), reinterpret_cast<std::byte *>(this->{}.path.data()) + {}_size + 1);\n".format(name, name, name))
                cpp_save_hek_data.write("        }\n")
            elif struct["type"] == "TagReflexive":
                cpp_save_hek_data.write("        auto ref_{}_size = this->{}.size();\n".format(name, name))
                cpp_save_hek_data.write("        if(ref_{}_size > 0) {{\n".format(name))
                cpp_save_hek_data.write("            b.{}.count = static_cast<std::uint32_t>(ref_{}_size);\n".format(name, name))
                cpp_save_hek_data.write("            constexpr std::size_t STRUCT_SIZE = sizeof({}::struct_big);\n".format(struct["struct"]))
                cpp_save_hek_data.write("            auto total_size = STRUCT_SIZE * ref_{}_size;\n".format(name))
                cpp_save_hek_data.write("            const std::size_t FIRST_STRUCT_OFFSET = converted_data.size();\n")
                cpp_save_hek_data.write("            converted_data.insert(converted_data.end(), total_size, std::byte());\n")
                cpp_save_hek_data.write("            for(std::size_t i = 0; i < ref_{}_size; i++) {{\n".format(name))
                cpp_save_hek_data.write("                const auto converted_struct = this->{}[i].generate_hek_tag_data(std::nullopt, clear_on_save);\n".format(name))
                cpp_save_hek_data.write("                const auto *struct_data = converted_struct.data();\n")
                cpp_save_hek_data.write("                std::copy(struct_data, struct_data + STRUCT_SIZE, converted_data.data() + FIRST_STRUCT_OFFSET + STRUCT_SIZE * i);\n")
                cpp_save_hek_data.write("                converted_data.insert(converted_data.end(), struct_data + STRUCT_SIZE, struct_data + converted_struct.size());\n")
                cpp_save_hek_data.write("            }\n")
                cpp_save_hek_data.write("            if(clear_on_save) {\n")
                cpp_save_hek_data.write("                this->{} = std::vector<{}>();\n".format(name, struct["struct"]))
                cpp_save_hek_data.write("            }\n")
                cpp_save_hek_data.write("        }\n")
            elif struct["type"] == "TagDataOffset":
                cpp_save_hek_data.write("        b.{}.size = static_cast<std::uint32_t>(this->{}.size());\n".format(name, name))
                cpp_save_hek_data.write("        converted_data.insert(converted_data.end(), this->{}.begin(), this->{}.end());\n".format(name, name, name))
                cpp_save_hek_data.write("        if(clear_on_save) {\n")
                cpp_save_hek_data.write("            this->{} = std::vector<std::byte>();\n".format(name))
                cpp_save_hek_data.write("        }\n")
            elif "bounds" in struct and struct["bounds"]:
                cpp_save_hek_data.write("        b.{}.from = this->{}.from;\n".format(name, name))
                cpp_save_hek_data.write("        b.{}.to = this->{}.to;\n".format(name, name))
            elif "count" in struct and struct["count"] > 1:
                cpp_save_hek_data.write("        std::copy(this->{}, this->{} + {}, b.{});\n".format(name, name, struct["count"], name))
            else:
                cpp_save_hek_data.write("        b.{} = this->{};\n".format(name, name))
        cpp_save_hek_data.write("        *reinterpret_cast<struct_big *>(converted_data.data() + tag_header_offset) = b;\n")
    cpp_save_hek_data.write("        if(generate_header_class.has_value()) {\n")
    cpp_save_hek_data.write("            reinterpret_cast<HEK::TagFileHeader *>(converted_data.data())->crc32 = ~crc32(clear_on_save ^ clear_on_save, reinterpret_cast<const void *>(converted_data.data() + tag_header_offset), converted_data.size() - tag_header_offset);\n")
    cpp_save_hek_data.write("        }\n")
    cpp_save_hek_data.write("        return converted_data;\n")
    cpp_save_hek_data.write("    }\n")

    # parse_cache_file_data()
    hpp.write("\n        /**\n")
    hpp.write("         * Parse the cache file tag data.\n")
    hpp.write("         * @param tag     Tag to read data from\n")
    hpp.write("         * @param pointer Pointer to read from; if none is given, then the start of the tag will be used\n")
    hpp.write("         * @return parsed tag data\n")
    hpp.write("         */\n")
    hpp.write("        static {} parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer = std::nullopt);\n".format(struct_name))
    if len(all_used_structs) > 0 or post_cache_parse:
        cpp_read_cache_file_data.write("    {} {}::parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {{\n".format(struct_name, struct_name))
    else:
        cpp_read_cache_file_data.write("    {} {}::parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {{\n".format(struct_name, struct_name))
    cpp_read_cache_file_data.write("        {} r = {{}};\n".format(struct_name))
    cpp_read_cache_file_data.write("        r.cache_formatted = true;\n")
    if len(all_used_structs) > 0:
        cpp_read_cache_file_data.write("        const auto &l = pointer.has_value() ? tag.get_struct_at_pointer<HEK::{}>(*pointer) : tag.get_base_struct<HEK::{}>();\n".format(struct_name, struct_name))
        for struct in all_used_structs:
            name = struct["name"]
            if struct["type"] == "TagID":
                cpp_read_cache_file_data.write("        r.{} = HEK::TagID::null_tag_id();\n".format(name))
                continue
            if ("non_cached" in struct and struct["non_cached"]) or ("ignore_cached" in struct and struct["ignore_cached"]):
                continue
            if struct["type"] == "TagDependency":
                cpp_read_cache_file_data.write("        r.{}.tag_class_int = l.{}.tag_class_int.read();\n".format(name, name))
                cpp_read_cache_file_data.write("        r.{}.tag_id = l.{}.tag_id.read();\n".format(name, name))
                cpp_read_cache_file_data.write("        if(!r.{}.tag_id.is_null()) {{\n".format(name))
                cpp_read_cache_file_data.write("            try {\n")
                cpp_read_cache_file_data.write("                auto &referenced_tag = tag.get_map().get_tag(r.{}.tag_id.index);\n".format(name))
                cpp_read_cache_file_data.write("                if(referenced_tag.get_tag_class_int() != r.{}.tag_class_int) {{\n".format(name))
                cpp_read_cache_file_data.write("                    eprintf_error(\"Corrupt tag reference (class in reference does not match class in referenced tag)\");\n")
                cpp_read_cache_file_data.write("                    throw InvalidTagDataException();\n")
                cpp_read_cache_file_data.write("                }\n")
                cpp_read_cache_file_data.write("                r.{}.path = referenced_tag.get_path();\n".format(name))
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("            catch (std::exception &) {\n")
                cpp_read_cache_file_data.write("                eprintf_error(\"Invalid reference for {}.{} in %s.%s\", File::halo_path_to_preferred_path(tag.get_path()).data(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
                cpp_read_cache_file_data.write("                throw;\n")
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("            for(char &c : r.{}.path) {{\n".format(name))
                cpp_read_cache_file_data.write("                c = std::tolower(c);\n")
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("        }\n")
            elif struct["type"] == "TagReflexive":
                cpp_read_cache_file_data.write("        std::size_t l_{}_count = l.{}.count.read();\n".format(name, name))
                cpp_read_cache_file_data.write("        r.{}.reserve(l_{}_count);\n".format(name, name))
                cpp_read_cache_file_data.write("        if(l_{}_count > 0) {{\n".format(name))
                cpp_read_cache_file_data.write("            auto l_{}_ptr = l.{}.pointer;\n".format(name, name))
                cpp_read_cache_file_data.write("            for(std::size_t i = 0; i < l_{}_count; i++) {{\n".format(name))
                cpp_read_cache_file_data.write("                try {\n")
                cpp_read_cache_file_data.write("                    r.{}.emplace_back({}::parse_cache_file_data(tag, l_{}_ptr + i * sizeof({}::struct_little)));\n".format(name, struct["struct"], name, struct["struct"]))
                cpp_read_cache_file_data.write("                }\n")
                cpp_read_cache_file_data.write("                catch (std::exception &) {\n")
                cpp_read_cache_file_data.write("                    eprintf_error(\"Failed to parse {}.{} #%zu in %s.%s\", i, File::halo_path_to_preferred_path(tag.get_path()).data(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
                cpp_read_cache_file_data.write("                    throw;\n")
                cpp_read_cache_file_data.write("                }\n")
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("        }\n")
            elif struct["type"] == "TagDataOffset":
                cpp_read_cache_file_data.write("        std::size_t l_{}_data_size = l.{}.size;\n".format(name, name))
                cpp_read_cache_file_data.write("        if(l_{}_data_size > 0) {{\n".format(name))
                cpp_read_cache_file_data.write("            const std::byte *data;\n")
                cpp_read_cache_file_data.write("            try {\n")
                if "file_offset" in struct:
                    if "external_file_offset" in struct:
                        where_to = "DATA_MAP_CACHE"
                        if struct["external_file_offset"] == "sounds.map":
                            where_to = "DATA_MAP_SOUND"
                        elif struct["external_file_offset"] == "bitmaps.map":
                            where_to = "DATA_MAP_BITMAP"
                        elif struct["external_file_offset"] == "loc.map":
                            where_to = "DATA_MAP_LOC"
                        else:
                            print("Unknown external_file_offset: {}".format(struct["external_file_offset"]), file=sys.stderr)
                            sys.exit(1)
                        cpp_read_cache_file_data.write("                data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size, (l.{}.external & 1) ? Map::DataMapType::{} : Map::DataMapType::DATA_MAP_CACHE);\n".format(name, name, name, where_to))
                        pass
                    else:
                        cpp_read_cache_file_data.write("                data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size);\n".format(name, name))
                    pass
                else:
                    cpp_read_cache_file_data.write("                data = tag.data(l.{}.pointer, l_{}_data_size);\n".format(name, name))
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("            catch (std::exception &) {\n")
                cpp_read_cache_file_data.write("                eprintf_error(\"Failed to read tag data for {}.{} in %s.%s\", tag.get_path().data(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
                cpp_read_cache_file_data.write("                throw;\n")
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("            r.{}.insert(r.{}.begin(), data, data + l_{}_data_size);\n".format(name, name, name))
                cpp_read_cache_file_data.write("        }\n")
            elif "bounds" in struct and struct["bounds"]:
                cpp_read_cache_file_data.write("        r.{}.from = l.{}.from;\n".format(name, name))
                cpp_read_cache_file_data.write("        r.{}.to = l.{}.to;\n".format(name, name))
            elif "count" in struct and struct["count"] > 1:
                cpp_read_cache_file_data.write("        std::copy(l.{}, l.{} + {}, r.{});\n".format(name, name, struct["count"], name))
            else:
                cpp_read_cache_file_data.write("        r.{} = l.{};\n".format(name, name))
    if post_cache_parse:
        cpp_read_cache_file_data.write("        r.post_cache_parse(tag, pointer);\n")
    cpp_read_cache_file_data.write("        return r;\n")
    cpp_read_cache_file_data.write("    }\n")

    # parse_hek_tag_data()
    hpp.write("\n        /**\n")
    hpp.write("         * Parse the HEK tag data.\n")
    hpp.write("         * @param data      Data to read from for structs, tag references, and reflexives; if data_this is nullptr, this must point to the struct\n")
    hpp.write("         * @param data_size Size of the buffer\n")
    hpp.write("         * @param data_read This will be set to the amount of data read. If data_this is null, then the initial struct will also be added\n")
    hpp.write("         * @param data_this Pointer to the struct; if this is null, then data will be used instead\n")
    hpp.write("         * @return parsed tag data\n")
    hpp.write("         */\n")
    hpp.write("        static {} parse_hek_tag_data(const std::byte *data, std::size_t data_size, std::size_t &data_read, const std::byte *data_this = nullptr);\n".format(struct_name))
    cpp_read_hek_data.write("    {} {}::parse_hek_tag_data(const std::byte *data, std::size_t data_size, std::size_t &data_read, const std::byte *data_this) {{\n".format(struct_name, struct_name))
    cpp_read_hek_data.write("        {} r = {{}};\n".format(struct_name))
    cpp_read_hek_data.write("        data_read = 0;\n")
    cpp_read_hek_data.write("        if(data_this == nullptr) {\n")
    cpp_read_hek_data.write("            if(sizeof(struct_big) > data_size) {\n")
    cpp_read_hek_data.write("                eprintf_error(\"Failed to read {} base struct: %zu bytes needed > %zu bytes available\", sizeof(struct_big), data_size);\n".format(struct_name))
    cpp_read_hek_data.write("                throw OutOfBoundsException();\n")
    cpp_read_hek_data.write("            }\n")
    cpp_read_hek_data.write("            data_this = data;\n")
    cpp_read_hek_data.write("            data_size -= sizeof(struct_big);\n")
    cpp_read_hek_data.write("            data_read += sizeof(struct_big);\n")
    cpp_read_hek_data.write("            data += sizeof(struct_big);\n")
    cpp_read_hek_data.write("        }\n")
    if len(all_used_structs) > 0:
        cpp_read_hek_data.write("        const auto &h = *reinterpret_cast<const HEK::{}<HEK::BigEndian> *>(data_this);\n".format(struct_name))
        for struct in all_used_structs:
            name = struct["name"]
            if struct["type"] == "TagDependency":
                cpp_read_hek_data.write("        std::size_t h_{}_expected_length = h.{}.path_size;\n".format(name,name))
                cpp_read_hek_data.write("        r.{}.tag_class_int = h.{}.tag_class_int;\n".format(name, name))
                cpp_read_hek_data.write("        if(h_{}_expected_length > 0) {{\n".format(name))
                cpp_read_hek_data.write("            if(h_{}_expected_length + 1 > data_size) {{\n".format(name))
                cpp_read_hek_data.write("                eprintf_error(\"Failed to read dependency {}.{}: %zu bytes needed > %zu bytes available\", h_{}_expected_length, data_size);\n".format(struct_name, name, name))
                cpp_read_hek_data.write("                throw OutOfBoundsException();\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            const char *h_{}_char = reinterpret_cast<const char *>(data);\n".format(name))
                cpp_read_hek_data.write("            for(std::size_t i = 0; i < h_{}_expected_length; i++) {{\n".format(name))
                cpp_read_hek_data.write("                if(h_{}_char[i] == 0) {{\n".format(name))
                cpp_read_hek_data.write("                    eprintf_error(\"Failed to read dependency {}.{}: size is smaller than expected (%zu expected > %zu actual)\", h_{}_expected_length, i);\n".format(struct_name, name, name))
                cpp_read_hek_data.write("                    throw InvalidTagDataException();\n")
                cpp_read_hek_data.write("                }\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            if(static_cast<char>(data[h_{}_expected_length]) != 0) {{\n".format(name))
                cpp_read_hek_data.write("                eprintf_error(\"Failed to read dependency {}.{}: missing null terminator\");\n".format(struct_name, name))
                cpp_read_hek_data.write("                throw InvalidTagDataException();\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            r.{}.path = std::string(reinterpret_cast<const char *>(data));\n".format(name))
                cpp_read_hek_data.write("            data_size -= h_{}_expected_length + 1;\n".format(name))
                cpp_read_hek_data.write("            data_read += h_{}_expected_length + 1;\n".format(name))
                cpp_read_hek_data.write("            data += h_{}_expected_length + 1;\n".format(name))
                cpp_read_hek_data.write("        }\n")
            elif struct["type"] == "TagReflexive":
                cpp_read_hek_data.write("        std::size_t h_{}_count = h.{}.count;\n".format(name,name))
                cpp_read_hek_data.write("        if(h_{}_count > 0) {{\n".format(name))
                cpp_read_hek_data.write("            const auto *array = reinterpret_cast<const HEK::{}<HEK::BigEndian> *>(data);\n".format(struct["struct"]))
                cpp_read_hek_data.write("            std::size_t total_size = sizeof(*array) * h_{}_count;\n".format(name))
                cpp_read_hek_data.write("            if(total_size > data_size) {\n")
                cpp_read_hek_data.write("                eprintf_error(\"Failed to read reflexive {}.{}: %zu bytes needed > %zu bytes available\", total_size, data_size);\n".format(struct_name, name))
                cpp_read_hek_data.write("                throw OutOfBoundsException();\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            data_size -= total_size;\n")
                cpp_read_hek_data.write("            data_read += total_size;\n")
                cpp_read_hek_data.write("            data += total_size;\n")
                cpp_read_hek_data.write("            r.{}.reserve(h_{}_count);\n".format(name, name))
                cpp_read_hek_data.write("            for(std::size_t ref = 0; ref < h_{}_count; ref++) {{\n".format(name))
                cpp_read_hek_data.write("                std::size_t ref_data_read = 0;\n")
                cpp_read_hek_data.write("                r.{}.emplace_back({}::parse_hek_tag_data(data, data_size, ref_data_read, reinterpret_cast<const std::byte *>(array + ref)));\n".format(name, struct["struct"]))
                cpp_read_hek_data.write("                data += ref_data_read;\n")
                cpp_read_hek_data.write("                data_read += ref_data_read;\n")
                cpp_read_hek_data.write("                data_size -= ref_data_read;\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("        }\n")
            elif struct["type"] == "TagDataOffset":
                cpp_read_hek_data.write("        std::size_t h_{}_size = h.{}.size;\n".format(name, name))
                cpp_read_hek_data.write("        if(h_{}_size > data_size) {{\n".format(name))
                cpp_read_hek_data.write("            eprintf_error(\"Failed to read tag data block {}.{}: %zu bytes needed > %zu bytes available\", h_{}_size, data_size);\n".format(struct_name, name, name))
                cpp_read_hek_data.write("            throw OutOfBoundsException();\n")
                cpp_read_hek_data.write("        }\n")
                cpp_read_hek_data.write("        r.{} = std::vector<std::byte>(data, data + h_{}_size);\n".format(name, name))
                cpp_read_hek_data.write("        data_size -= h_{}_size;\n".format(name))
                cpp_read_hek_data.write("        data_read += h_{}_size;\n".format(name))
                cpp_read_hek_data.write("        data += h_{}_size;\n".format(name))
            elif struct["type"] == "ColorRGB":
                cpp_read_hek_data.write("        r.{} = h.{};\n".format(name, name))
                if "default" in struct:
                    default = struct["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    cpp_read_hek_data.write("        if(r.{}.red == 0 && r.{}.green == 0 && r.{}.blue == 0) {{\n".format(name,name,name))
                    cpp_read_hek_data.write("            r.{}.red = {}{};\n".format(name, default[0], suffix))
                    cpp_read_hek_data.write("            r.{}.green = {}{};\n".format(name, default[1], suffix))
                    cpp_read_hek_data.write("            r.{}.blue = {}{};\n".format(name, default[2], suffix))
                    cpp_read_hek_data.write("        }\n")
            elif struct["type"] == "ColorARGB" or struct["type"] == "ColorARGBInt":
                cpp_read_hek_data.write("        r.{} = h.{};\n".format(name, name))
                if "default" in struct:
                    default = struct["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    cpp_read_hek_data.write("        if(r.{}.alpha == 0 && r.{}.red == 0 && r.{}.green == 0 && r.{}.blue == 0) {{\n".format(name,name,name,name))
                    cpp_read_hek_data.write("            r.{}.alpha = {}{};\n".format(name, default[0], suffix))
                    cpp_read_hek_data.write("            r.{}.red = {}{};\n".format(name, default[1], suffix))
                    cpp_read_hek_data.write("            r.{}.green = {}{};\n".format(name, default[2], suffix))
                    cpp_read_hek_data.write("            r.{}.blue = {}{};\n".format(name, default[3], suffix))
                    cpp_read_hek_data.write("        }\n")
            elif struct["type"] == "TagID":
                cpp_read_hek_data.write("        r.{} = HEK::TagID::null_tag_id();\n".format(name))
            elif "bounds" in struct and struct["bounds"]:
                cpp_read_hek_data.write("        r.{}.from = h.{}.from;\n".format(name, name))
                cpp_read_hek_data.write("        r.{}.to = h.{}.to;\n".format(name, name))
                if "default" in struct:
                    default = struct["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    cpp_read_hek_data.write("        if(r.{}.from == 0 && r.{}.to == 0) {{\n".format(name, name))
                    cpp_read_hek_data.write("            r.{}.from = {}{};\n".format(name, default[0], suffix))
                    cpp_read_hek_data.write("            r.{}.to = {}{};\n".format(name, default[1], suffix))
                    cpp_read_hek_data.write("        }\n")
            elif "count" in struct and struct["count"] > 1:
                cpp_read_hek_data.write("        std::copy(h.{}, h.{} + {}, r.{});\n".format(name, name, struct["count"], name))
                if "default" in struct:
                    default = struct["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    for q in range(struct["count"]):
                        cpp_read_hek_data.write("        if(r.{}[{}] == 0) {{\n".format(name, q))
                        cpp_read_hek_data.write("            r.{}[{}] = {}{};\n".format(name, q, default[q], suffix))
                        cpp_read_hek_data.write("        }\n")
            else:
                cpp_read_hek_data.write("        r.{} = h.{};\n".format(name, name))
                if "default" in struct:
                    default = struct["default"]
                    suffix = "F" if isinstance(default, float) else ""
                    cpp_read_hek_data.write("        if(r.{} == 0) {{\n".format(name))
                    cpp_read_hek_data.write("            r.{} = {}{};\n".format(name, default, suffix))
                    cpp_read_hek_data.write("        }\n")
    cpp_read_hek_data.write("        return r;\n")
    cpp_read_hek_data.write("    }\n")

    # parse_hek_tag_file()
    hpp.write("\n        /**\n")
    hpp.write("         * Parse the HEK tag file.\n")
    hpp.write("         * @param data      Tag file data to read from\n")
    hpp.write("         * @param data_size Size of the tag file\n")
    hpp.write("         * @return parsed tag data\n")
    hpp.write("         */\n")
    hpp.write("        static {} parse_hek_tag_file(const std::byte *data, std::size_t data_size);\n".format(struct_name))
    cpp_read_hek_data.write("    {} {}::parse_hek_tag_file(const std::byte *data, std::size_t data_size) {{\n".format(struct_name, struct_name))
    cpp_read_hek_data.write("        HEK::TagFileHeader::validate_header(reinterpret_cast<const HEK::TagFileHeader *>(data), data_size, true);\n")
    cpp_read_hek_data.write("        std::size_t data_read = 0;\n")
    cpp_read_hek_data.write("        std::size_t expected_data_read = data_size - sizeof(HEK::TagFileHeader);\n")
    cpp_read_hek_data.write("        auto r = parse_hek_tag_data(data + sizeof(HEK::TagFileHeader), expected_data_read, data_read);\n")
    cpp_read_hek_data.write("        if(data_read != expected_data_read) {\n")
    cpp_read_hek_data.write("            eprintf_error(\"invalid tag file; tag data was left over\");")
    cpp_read_hek_data.write("            throw InvalidTagDataException();\n")
    cpp_read_hek_data.write("        }\n")
    cpp_read_hek_data.write("        return r;\n")
    cpp_read_hek_data.write("    }\n")

    hpp.write("    private:\n")
    hpp.write("    bool cache_formatted = false;\n")

    if post_cache_deformat:
        hpp.write("    void post_cache_deformat();\n")

    if post_cache_parse:
        hpp.write("    void post_cache_parse(const Invader::Tag &, std::optional<HEK::Pointer>);\n")

    if pre_compile:
        hpp.write("    void pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);\n")

    if post_compile:
        hpp.write("    void post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);\n")

    hpp.write("    };\n")
hpp.write("}\n")
hpp.write("#endif\n")
write_for_all_cpps("}\n")
hpp.close()
cpp_save_hek_data.close()
cpp_read_cache_file_data.close()
cpp_read_hek_data.close()
cpp_cache_format_data.close()
cpp_cache_deformat_data.close()
