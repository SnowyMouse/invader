# SPDX-License-Identifier: GPL-3.0-only

from compile import make_cache_format_data
from generate_hek_tag_data import make_cpp_save_hek_data
from read_cache_file_data import make_parse_cache_file_data

def make_parser(all_enums, all_bitfields, all_structs_arranged, all_structs, extract_hidden, hpp, cpp_save_hek_data, cpp_read_cache_file_data, cpp_read_hek_data, cpp_cache_format_data, cpp_cache_deformat_data):
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
    hpp.write("#include \"parser_struct.hpp\"\n\n")
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
        postprocess_hek_data = "postprocess_hek_data" in s and s["postprocess_hek_data"]
        private_functions = post_cache_deformat

        hpp.write("    struct {} : public ParserStruct {{\n".format(struct_name))
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
        hpp.write("         * Format the tag to be used in HEK tags.\n")
        hpp.write("         */\n")
        hpp.write("        void cache_deformat() override;\n")
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

        make_cache_format_data(struct_name, s, pre_compile, post_compile, all_used_structs, hpp, cpp_cache_format_data)
        make_cpp_save_hek_data(extract_hidden, all_used_structs, struct_name, hpp, cpp_save_hek_data)
        make_parse_cache_file_data(post_cache_parse, all_used_structs, struct_name, hpp, cpp_read_cache_file_data)

        # parse_hek_tag_data()
        hpp.write("\n        /**\n")
        hpp.write("         * Parse the HEK tag data.\n")
        hpp.write("         * @param data        Data to read from for structs, tag references, and reflexives; if data_this is nullptr, this must point to the struct\n")
        hpp.write("         * @param data_size   Size of the buffer\n")
        hpp.write("         * @param data_read   This will be set to the amount of data read. If data_this is null, then the initial struct will also be added\n")
        hpp.write("         * @param postprocess Do post-processing on data, such as default values\n")
        hpp.write("         * @param data_this   Pointer to the struct; if this is null, then data will be used instead\n")
        hpp.write("         * @return parsed tag data\n")
        hpp.write("         */\n")
        hpp.write("        static {} parse_hek_tag_data(const std::byte *data, std::size_t data_size, std::size_t &data_read, bool postprocess = false, const std::byte *data_this = nullptr);\n".format(struct_name))
        cpp_read_hek_data.write("    {} {}::parse_hek_tag_data(const std::byte *data, std::size_t data_size, std::size_t &data_read, [[maybe_unused]] bool postprocess, const std::byte *data_this) {{\n".format(struct_name, struct_name))
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
                default_sign = "<=" if "default_sign" in struct and struct["default_sign"] else "=="
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
                    cpp_read_hek_data.write("                r.{}.emplace_back({}::parse_hek_tag_data(data, data_size, ref_data_read, postprocess, reinterpret_cast<const std::byte *>(array + ref)));\n".format(name, struct["struct"]))
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
                        cpp_read_hek_data.write("        if(postprocess && r.{}.red {} 0 && r.{}.green {} 0 && r.{}.blue {} 0) {{\n".format(name,default_sign,name,default_sign,name,default_sign))
                        cpp_read_hek_data.write("            r.{}.red = {}{};\n".format(name, default[0], suffix))
                        cpp_read_hek_data.write("            r.{}.green = {}{};\n".format(name, default[1], suffix))
                        cpp_read_hek_data.write("            r.{}.blue = {}{};\n".format(name, default[2], suffix))
                        cpp_read_hek_data.write("        }\n")
                elif struct["type"] == "ColorARGB" or struct["type"] == "ColorARGBInt":
                    cpp_read_hek_data.write("        r.{} = h.{};\n".format(name, name))
                    if "default" in struct:
                        default = struct["default"]
                        suffix = "F" if isinstance(default[0], float) else ""
                        cpp_read_hek_data.write("        if(postprocess && r.{}.alpha {} 0 && r.{}.red {} 0 && r.{}.green {} 0 && r.{}.blue {} 0) {{\n".format(name,default_sign,name,default_sign,name,default_sign,name,default_sign))
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
                        cpp_read_hek_data.write("        if(postprocess && r.{}.from {} 0 && r.{}.to {} 0) {{\n".format(name, default_sign, name, default_sign))
                        cpp_read_hek_data.write("            r.{}.from = {}{};\n".format(name, default[0], suffix))
                        cpp_read_hek_data.write("            r.{}.to = {}{};\n".format(name, default[1], suffix))
                        cpp_read_hek_data.write("        }\n")
                elif "count" in struct and struct["count"] > 1:
                    cpp_read_hek_data.write("        std::copy(h.{}, h.{} + {}, r.{});\n".format(name, name, struct["count"], name))
                    if "default" in struct:
                        default = struct["default"]
                        suffix = "F" if isinstance(default[0], float) else ""
                        for q in range(struct["count"]):
                            cpp_read_hek_data.write("        if(postprocess && r.{}[{}] {} 0) {{\n".format(name, q, default_sign))
                            cpp_read_hek_data.write("            r.{}[{}] = {}{};\n".format(name, q, default[q], suffix))
                            cpp_read_hek_data.write("        }\n")
                else:
                    cpp_read_hek_data.write("        r.{} = h.{};\n".format(name, name))
                    if "default" in struct:
                        default = struct["default"]
                        suffix = "F" if isinstance(default, float) else ""
                        cpp_read_hek_data.write("        if(postprocess && r.{} {} 0) {{\n".format(name, default_sign))
                        cpp_read_hek_data.write("            r.{} = {}{};\n".format(name, default, suffix))
                        cpp_read_hek_data.write("        }\n")
        if postprocess_hek_data:
            cpp_read_hek_data.write("        if(postprocess) {\n")
            cpp_read_hek_data.write("            r.postprocess_hek_data();\n")
            cpp_read_hek_data.write("        }\n")
        cpp_read_hek_data.write("        return r;\n")
        cpp_read_hek_data.write("    }\n")

        # parse_hek_tag_file()
        hpp.write("\n        /**\n")
        hpp.write("         * Parse the HEK tag file.\n")
        hpp.write("         * @param data        Tag file data to read from\n")
        hpp.write("         * @param data_size   Size of the tag file\n")
        hpp.write("         * @param postprocess Do post-processing on data, such as default values\n")
        hpp.write("         * @return parsed tag data\n")
        hpp.write("         */\n")
        hpp.write("        static {} parse_hek_tag_file(const std::byte *data, std::size_t data_size, bool postprocess = false);\n".format(struct_name))
        cpp_read_hek_data.write("    {} {}::parse_hek_tag_file(const std::byte *data, std::size_t data_size, bool postprocess) {{\n".format(struct_name, struct_name))
        cpp_read_hek_data.write("        HEK::TagFileHeader::validate_header(reinterpret_cast<const HEK::TagFileHeader *>(data), data_size);\n")
        cpp_read_hek_data.write("        std::size_t data_read = 0;\n")
        cpp_read_hek_data.write("        std::size_t expected_data_read = data_size - sizeof(HEK::TagFileHeader);\n")
        cpp_read_hek_data.write("        auto r = parse_hek_tag_data(data + sizeof(HEK::TagFileHeader), expected_data_read, data_read, postprocess);\n")
        cpp_read_hek_data.write("        if(data_read != expected_data_read) {\n")
        cpp_read_hek_data.write("            eprintf_error(\"invalid tag file; tag data was left over\");")
        cpp_read_hek_data.write("            throw InvalidTagDataException();\n")
        cpp_read_hek_data.write("        }\n")
        cpp_read_hek_data.write("        return r;\n")
        cpp_read_hek_data.write("    }\n")

        hpp.write("        ~{}() override = default;\n".format(struct_name))
        hpp.write("    private:\n")

        if postprocess_hek_data:
            hpp.write("        void postprocess_hek_data();\n")

        if post_cache_deformat:
            hpp.write("        void post_cache_deformat();\n")

        if post_cache_parse:
            hpp.write("        void post_cache_parse(const Invader::Tag &, std::optional<HEK::Pointer>);\n")

        if pre_compile:
            hpp.write("        void pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);\n")

        if post_compile:
            hpp.write("        void post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);\n")

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
