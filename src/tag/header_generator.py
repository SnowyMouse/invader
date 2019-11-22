# SPDX-License-Identifier: GPL-3.0-only

import sys
import json
import os

if len(sys.argv) < 4:
    print("Usage: {} <definition.hpp> <parser.hpp> <parser.cpp> <json> [json [...]]".format(sys.argv[0]), file=sys.stderr)
    sys.exit(1)

files = []
all_enums = []
all_bitfields = []
all_structs = []

for i in range(4, len(sys.argv)):
    def make_name_fun(name, ignore_numbers):
        name = name.replace(" ", "_").replace("'", "")
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

        for n in range(0,len(e["options"])):
            f.write("        {}_{}{}\n".format(prefix,e["options"][n].upper(), "," if n + 1 < len(e["options"]) else ""))

        f.write("    };\n")

    for b in all_bitfields:
        f.write("    struct {} {{\n".format(b["name"]))
        for q in b["fields"]:
            f.write("        std::uint{}_t {} : 1;\n".format(b["width"], q))
        f.write("    };\n")

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
        f.write("        ENDIAN_TEMPLATE(NewEndian) operator {}<NewEndian>() const noexcept {{\n".format(s["name"]))
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
cpp = open(sys.argv[3], "w")
hpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
cpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
header_name = "INVADER__TAG__PARSER__PARSER_HPP"
hpp.write("#ifndef {}\n".format(header_name))
hpp.write("#define {}\n\n".format(header_name))
hpp.write("#include <string>\n")
hpp.write("#include <optional>\n")
hpp.write("#include \"../../map/map.hpp\"\n")
hpp.write("#include \"../hek/definition.hpp\"\n\n")
hpp.write("namespace Invader::Parser {\n")
hpp.write("    struct Dependency {\n")
hpp.write("        TagClassInt tag_class_int;\n")
hpp.write("        std::string path;\n")
hpp.write("        HEK::TagID tag_id = HEK::TagID::null_tag_id();\n")
hpp.write("    };\n")

cpp.write("#include <invader/tag/parser/parser.hpp>\n")
cpp.write("#include <invader/map/map.hpp>\n")
cpp.write("#include <invader/map/tag.hpp>\n")
cpp.write("#include <invader/tag/hek/header.hpp>\n")
cpp.write("#include <invader/printf.hpp>\n")
cpp.write("extern \"C\" std::uint32_t crc32(std::uint32_t crc, const void *buf, std::size_t size) noexcept;\n")
cpp.write("namespace Invader::Parser {\n")

for s in all_structs_arranged:
    struct_name = s["name"]
    post_parse_cache_file_data = "post_parse_cache_file_data" in s and s["post_parse_cache_file_data"]
    private_functions = post_parse_cache_file_data

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

    # generate_hek_tag_data()
    hpp.write("\n        /**\n")
    hpp.write("         * Convert the struct into HEK tag data to be built into a cache file.\n")
    hpp.write("         * @param  generate_header_class generate a cache file header with the class, too\n")
    hpp.write("         * @return cache file data\n")
    hpp.write("         */\n")
    hpp.write("        std::vector<std::byte> generate_hek_tag_data(std::optional<TagClassInt> generate_header_class = std::nullopt);\n")

    cpp.write("    std::vector<std::byte> {}::generate_hek_tag_data(std::optional<TagClassInt> generate_header_class) {{\n".format(struct_name))
    cpp.write("        std::vector<std::byte> converted_data(sizeof(struct_big));\n")
    cpp.write("        std::size_t tag_header_offset = 0;\n")
    cpp.write("        if(generate_header_class.has_value()) {\n")
    cpp.write("            HEK::TagFileHeader header(*generate_header_class);\n")
    cpp.write("            tag_header_offset = sizeof(header);\n")
    cpp.write("            converted_data.insert(converted_data.begin(), reinterpret_cast<std::byte *>(&header), reinterpret_cast<std::byte *>(&header + 1));\n")
    cpp.write("        }\n")
    if len(all_used_structs) > 0:
        cpp.write("        struct_big b = {};\n")
        for struct in all_used_structs:
            if "cache_only" in struct and struct["cache_only"]:
                continue
            name = struct["name"]
            if struct["type"] == "TagDependency":
                cpp.write("        std::size_t {}_size = static_cast<std::uint32_t>(this->{}.path.size());\n".format(name,name))
                cpp.write("        b.{}.tag_class_int = this->{}.tag_class_int;\n".format(name, name))
                cpp.write("        b.{}.tag_id = HEK::TagID::null_tag_id();\n".format(name))
                cpp.write("        if({}_size > 0) {{\n".format(name))
                cpp.write("            b.{}.path_size = static_cast<std::uint32_t>({}_size);\n".format(name, name))
                cpp.write("            converted_data.insert(converted_data.end(), reinterpret_cast<std::byte *>(this->{}.path.data()), reinterpret_cast<std::byte *>(this->{}.path.data()) + {}_size + 1);\n".format(name, name, name))
                cpp.write("        }\n")
            elif struct["type"] == "TagReflexive":
                cpp.write("        auto ref_{}_size = this->{}.size();\n".format(name, name))
                cpp.write("        if(ref_{}_size > 0) {{\n".format(name))
                cpp.write("            b.{}.count = static_cast<std::uint32_t>(ref_{}_size);\n".format(name, name))
                cpp.write("            constexpr std::size_t STRUCT_SIZE = sizeof({}::struct_big);\n".format(struct["struct"]))
                cpp.write("            auto total_size = STRUCT_SIZE * ref_{}_size;\n".format(name))
                cpp.write("            const std::size_t FIRST_STRUCT_OFFSET = converted_data.size();\n")
                cpp.write("            converted_data.insert(converted_data.end(), total_size, std::byte());\n")
                cpp.write("            for(std::size_t i = 0; i < ref_{}_size; i++) {{\n".format(name))
                cpp.write("                const auto converted_struct = this->{}[i].generate_hek_tag_data();\n".format(name))
                cpp.write("                const auto *struct_data = converted_struct.data();\n")
                cpp.write("                std::copy(struct_data, struct_data + STRUCT_SIZE, converted_data.data() + FIRST_STRUCT_OFFSET + STRUCT_SIZE * i);\n")
                cpp.write("                converted_data.insert(converted_data.end(), struct_data + STRUCT_SIZE, struct_data + converted_struct.size());\n")
                cpp.write("            }\n")
                cpp.write("        }\n")
            elif struct["type"] == "TagDataOffset":
                cpp.write("        b.{}.size = static_cast<std::uint32_t>(this->{}.size());\n".format(name, name))
                cpp.write("        converted_data.insert(converted_data.end(), this->{}.begin(), this->{}.end());\n".format(name, name, name))
            elif "bounds" in struct and struct["bounds"]:
                cpp.write("        b.{}.from = this->{}.from;\n".format(name, name))
                cpp.write("        b.{}.to = this->{}.to;\n".format(name, name))
            elif "count" in struct and struct["count"] > 1:
                cpp.write("        std::copy(this->{}, this->{} + {}, b.{});\n".format(name, name, struct["count"], name))
            else:
                cpp.write("        b.{} = this->{};\n".format(name, name))
        cpp.write("        *reinterpret_cast<struct_big *>(converted_data.data() + tag_header_offset) = b;\n")
    cpp.write("        if(generate_header_class.has_value()) {\n")
    cpp.write("            reinterpret_cast<HEK::TagFileHeader *>(converted_data.data())->crc32 = ~crc32(0, reinterpret_cast<const void *>(converted_data.data() + tag_header_offset), converted_data.size() - tag_header_offset);\n")
    cpp.write("        }\n")
    cpp.write("        return converted_data;\n")
    cpp.write("    }\n")

    # parse_cache_file_data()
    hpp.write("\n        /**\n")
    hpp.write("         * Parse the cache file tag data.\n")
    hpp.write("         * @param tag     Tag to read data from\n")
    hpp.write("         * @param pointer Pointer to read from; if none is given, then the start of the tag will be used\n")
    hpp.write("         * @return parsed tag data\n")
    hpp.write("         */\n")
    hpp.write("        static {} parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer = std::nullopt);\n".format(struct_name))
    if len(all_used_structs) > 0 or post_parse_cache_file_data:
        cpp.write("    {} {}::parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {{\n".format(struct_name, struct_name))
    else:
        cpp.write("    {} {}::parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {{\n".format(struct_name, struct_name))
    cpp.write("        {} r = {{}};\n".format(struct_name))
    if len(all_used_structs) > 0:
        cpp.write("        const auto &l = pointer.has_value() ? tag.get_struct_at_pointer<HEK::{}>(*pointer) : tag.get_base_struct<HEK::{}>();\n".format(struct_name, struct_name))
        for struct in all_used_structs:
            name = struct["name"]
            if "non_cached" in struct and struct["non_cached"]:
                continue
            if struct["type"] == "TagDependency":
                cpp.write("        r.{}.tag_class_int = l.{}.tag_class_int.read();\n".format(name, name))
                cpp.write("        r.{}.tag_id = l.{}.tag_id.read();\n".format(name, name))
                cpp.write("        if(!r.{}.tag_id.is_null()) {{\n".format(name))
                cpp.write("            try {\n")
                cpp.write("                auto &referenced_tag = tag.get_map().get_tag(r.{}.tag_id.index);\n".format(name))
                cpp.write("                if(referenced_tag.get_tag_class_int() != r.{}.tag_class_int) {{\n".format(name))
                cpp.write("                    eprintf(\"corrupt tag reference (class in reference does not match class in referenced tag)\\n\");\n")
                cpp.write("                    throw InvalidTagDataException();\n")
                cpp.write("                }\n")
                cpp.write("                r.{}.path = referenced_tag.get_path();\n".format(name))
                cpp.write("            }\n")
                cpp.write("            catch (std::exception &) {\n")
                cpp.write("                eprintf(\"invalid reference for {}.{} in %s.%s\\n\", tag.get_path().data(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
                cpp.write("                throw;\n")
                cpp.write("            }\n")
                cpp.write("        }\n")
            elif struct["type"] == "TagReflexive":
                cpp.write("        std::size_t l_{}_count = l.{}.count.read();\n".format(name, name))
                cpp.write("        r.{}.reserve(l_{}_count);\n".format(name, name))
                cpp.write("        if(l_{}_count > 0) {{\n".format(name))
                cpp.write("            auto l_{}_ptr = l.{}.pointer;\n".format(name, name))
                cpp.write("            for(std::size_t i = 0; i < l_{}_count; i++) {{\n".format(name))
                cpp.write("                try {\n")
                cpp.write("                    r.{}.emplace_back({}::parse_cache_file_data(tag, l_{}_ptr + i * sizeof({}::struct_little)));\n".format(name, struct["struct"], name, struct["struct"]))
                cpp.write("                }\n")
                cpp.write("                catch (std::exception &) {\n")
                cpp.write("                    eprintf(\"failed to parse {}.{} #%zu in %s.%s\\n\", i, tag.get_path().data(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
                cpp.write("                    throw;\n")
                cpp.write("                }\n")
                cpp.write("            }\n")
                cpp.write("        }\n")
            elif struct["type"] == "TagDataOffset":
                cpp.write("        std::size_t l_{}_data_size = l.{}.size;\n".format(name, name))
                cpp.write("        if(l_{}_data_size > 0) {{\n".format(name))
                cpp.write("            const std::byte *data;\n")
                cpp.write("            try {\n")
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
                        cpp.write("                data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size, l.{}.external ? Map::DataMapType::{} : Map::DataMapType::DATA_MAP_CACHE);\n".format(name, name, name, where_to))
                        pass
                    else:
                        cpp.write("                data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size);\n".format(name, name))
                    pass
                else:
                    cpp.write("                data = tag.data(l.{}.pointer, l_{}_data_size);\n".format(name, name))
                cpp.write("            }\n")
                cpp.write("            catch (std::exception &) {\n")
                cpp.write("                eprintf(\"failed to read tag data for {}.{} in %s.%s\\n\", tag.get_path().data(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
                cpp.write("                throw;\n")
                cpp.write("            }\n")
                cpp.write("            r.{}.insert(r.{}.begin(), data, data + l_{}_data_size);\n".format(name, name, name))
                cpp.write("        }\n")
            elif "bounds" in struct and struct["bounds"]:
                cpp.write("        r.{}.from = l.{}.from;\n".format(name, name))
                cpp.write("        r.{}.to = l.{}.to;\n".format(name, name))
            elif "count" in struct and struct["count"] > 1:
                cpp.write("        std::copy(l.{}, l.{} + {}, r.{});\n".format(name, name, struct["count"], name))
            else:
                cpp.write("        r.{} = l.{};\n".format(name, name))
    if post_parse_cache_file_data:
        cpp.write("        r.post_parse_cache_file_data(tag, pointer);\n")
    cpp.write("        return r;\n")
    cpp.write("    }\n")

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
    cpp.write("    {} {}::parse_hek_tag_data(const std::byte *data, std::size_t data_size, std::size_t &data_read, const std::byte *data_this) {{\n".format(struct_name, struct_name))
    cpp.write("        {} r = {{}};\n".format(struct_name))
    cpp.write("        data_read = 0;\n")
    cpp.write("        if(data_this == nullptr) {\n")
    cpp.write("            if(sizeof(struct_big) > data_size) {\n")
    cpp.write("                eprintf(\"failed to read {} base struct: %zu bytes needed > %zu bytes available\\n\", sizeof(struct_big), data_size);\n".format(struct_name))
    cpp.write("                throw OutOfBoundsException();\n")
    cpp.write("            }\n")
    cpp.write("            data_this = data;\n")
    cpp.write("            data_size -= sizeof(struct_big);\n")
    cpp.write("            data_read += sizeof(struct_big);\n")
    cpp.write("            data += sizeof(struct_big);\n")
    cpp.write("        }\n")
    if len(all_used_structs) > 0:
        cpp.write("        const auto &h = *reinterpret_cast<const HEK::{}<HEK::BigEndian> *>(data_this);\n".format(struct_name))
        for struct in all_used_structs:
            name = struct["name"]
            if struct["type"] == "TagDependency":
                cpp.write("        std::size_t h_{}_expected_length = h.{}.path_size;\n".format(name,name))
                cpp.write("        r.{}.tag_class_int = h.{}.tag_class_int;\n".format(name, name))
                cpp.write("        if(h_{}_expected_length > 0) {{\n".format(name))
                cpp.write("            if(h_{}_expected_length + 1 > data_size) {{\n".format(name))
                cpp.write("                eprintf(\"failed to read dependency {}.{}: %zu bytes needed > %zu bytes available\\n\", h_{}_expected_length, data_size);\n".format(struct_name, name, name))
                cpp.write("                throw OutOfBoundsException();\n")
                cpp.write("            }\n")
                cpp.write("            const char *h_{}_char = reinterpret_cast<const char *>(data);\n".format(name))
                cpp.write("            for(std::size_t i = 0; i < h_{}_expected_length; i++) {{\n".format(name))
                cpp.write("                if(h_{}_char[i] == 0) {{\n".format(name))
                cpp.write("                    eprintf(\"failed to read dependency {}.{}: size is smaller than expected (%zu expected > %zu actual)\\n\", h_{}_expected_length, i);\n".format(struct_name, name, name))
                cpp.write("                    throw InvalidTagDataException();\n")
                cpp.write("                }\n")
                cpp.write("            }\n")
                cpp.write("            if(static_cast<char>(data[h_{}_expected_length]) != 0) {{\n".format(name))
                cpp.write("                eprintf(\"failed to read dependency {}.{}: missing null terminator\\n\");\n".format(struct_name, name))
                cpp.write("                throw InvalidTagDataException();\n")
                cpp.write("            }\n")
                cpp.write("            r.{}.path = std::string(reinterpret_cast<const char *>(data));\n".format(name))
                cpp.write("            data_size -= h_{}_expected_length + 1;\n".format(name))
                cpp.write("            data_read += h_{}_expected_length + 1;\n".format(name))
                cpp.write("            data += h_{}_expected_length + 1;\n".format(name))
                cpp.write("        }\n")
            elif struct["type"] == "TagReflexive":
                cpp.write("        std::size_t h_{}_count = h.{}.count;\n".format(name,name))
                cpp.write("        if(h_{}_count > 0) {{\n".format(name))
                cpp.write("            const auto *array = reinterpret_cast<const HEK::{}<HEK::BigEndian> *>(data);\n".format(struct["struct"]))
                cpp.write("            std::size_t total_size = sizeof(*array) * h_{}_count;\n".format(name))
                cpp.write("            if(total_size > data_size) {\n")
                cpp.write("                eprintf(\"failed to read reflexive {}.{}: %zu bytes needed > %zu bytes available\\n\", total_size, data_size);\n".format(struct_name, name))
                cpp.write("                throw OutOfBoundsException();\n")
                cpp.write("            }\n")
                cpp.write("            data_size -= total_size;\n")
                cpp.write("            data_read += total_size;\n")
                cpp.write("            data += total_size;\n")
                cpp.write("            r.{}.reserve(h_{}_count);\n".format(name, name))
                cpp.write("            for(std::size_t ref = 0; ref < h_{}_count; ref++) {{\n".format(name))
                cpp.write("                std::size_t ref_data_read = 0;\n")
                cpp.write("                r.{}.emplace_back({}::parse_hek_tag_data(data, data_size, ref_data_read, reinterpret_cast<const std::byte *>(array + ref)));\n".format(name, struct["struct"]))
                cpp.write("                data += ref_data_read;\n")
                cpp.write("                data_read += ref_data_read;\n")
                cpp.write("                data_size -= ref_data_read;\n")
                cpp.write("            }\n")
                cpp.write("        }\n")
            elif struct["type"] == "TagDataOffset":
                cpp.write("        std::size_t h_{}_size = h.{}.size;\n".format(name, name))
                cpp.write("        if(h_{}_size > data_size) {{\n".format(name))
                cpp.write("            eprintf(\"failed to read tag data block {}.{}: %zu bytes needed > %zu bytes available\\n\", h_{}_size, data_size);\n".format(struct_name, name, name))
                cpp.write("            throw OutOfBoundsException();\n")
                cpp.write("        }\n")
                cpp.write("        r.{} = std::vector<std::byte>(data, data + h_{}_size);\n".format(name, name))
                cpp.write("        data_size -= h_{}_size;\n".format(name))
                cpp.write("        data_read += h_{}_size;\n".format(name))
                cpp.write("        data += h_{}_size;\n".format(name))
            elif "bounds" in struct and struct["bounds"]:
                cpp.write("        r.{}.from = h.{}.from;\n".format(name, name))
                cpp.write("        r.{}.to = h.{}.to;\n".format(name, name))
            elif "count" in struct and struct["count"] > 1:
                cpp.write("        std::copy(h.{}, h.{} + {}, r.{});\n".format(name, name, struct["count"], name))
            else:
                cpp.write("        r.{} = h.{};\n".format(name, name))
    cpp.write("        return r;\n")
    cpp.write("    }\n")

    # parse_hek_tag_file()
    hpp.write("\n        /**\n")
    hpp.write("         * Parse the HEK tag file.\n")
    hpp.write("         * @param data      Tag file data to read from\n")
    hpp.write("         * @param data_size Size of the tag file\n")
    hpp.write("         * @return parsed tag data\n")
    hpp.write("         */\n")
    hpp.write("        static {} parse_hek_tag_file(const std::byte *data, std::size_t data_size);\n".format(struct_name))
    cpp.write("    {} {}::parse_hek_tag_file(const std::byte *data, std::size_t data_size) {{\n".format(struct_name, struct_name))
    cpp.write("        if(data_size < sizeof(HEK::TagFileHeader)) {\n")
    cpp.write("            eprintf(\"data is too small to have a header\\n\");")
    cpp.write("            throw OutOfBoundsException();\n")
    cpp.write("        }\n")
    cpp.write("        std::size_t data_read = 0;\n")
    cpp.write("        std::size_t expected_data_read = data_size - sizeof(HEK::TagFileHeader);\n")
    cpp.write("        auto r = parse_hek_tag_data(data + sizeof(HEK::TagFileHeader), expected_data_read, data_read);\n")
    cpp.write("        if(data_read != expected_data_read) {\n")
    cpp.write("            eprintf(\"invalid tag file; tag data was left over\\n\");")
    cpp.write("            throw InvalidTagDataException();\n")
    cpp.write("        }\n")
    cpp.write("        return r;\n")
    cpp.write("    }\n")

    if private_functions:
        hpp.write("    private:\n")
        if post_parse_cache_file_data:
            hpp.write("    void post_parse_cache_file_data(const Tag &tag, std::optional<HEK::Pointer> pointer);\n")

    hpp.write("    };\n")
hpp.write("}\n")
hpp.write("#endif\n")
cpp.write("}\n")
cpp.close()
hpp.close()
