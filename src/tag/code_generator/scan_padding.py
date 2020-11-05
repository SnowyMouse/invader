# SPDX-License-Identifier: GPL-3.0-only

import sys

def make_scan_padding(all_used_structs, struct_name, all_bitfields, hpp, cpp_scan_padding):
    hpp.write("\n        /**\n")
    hpp.write("         * Scan the padding for non-zero values.\n")
    hpp.write("         * @param tag     Tag to read data from\n")
    hpp.write("         * @param pointer Pointer to read from; if none is given, then the start of the tag will be used\n")
    hpp.write("         * @return parsed tag data\n")
    hpp.write("         */\n")
    hpp.write("        static void scan_padding(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer = std::nullopt);\n")
    cpp_scan_padding.write("    void {}::scan_padding(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {{\n".format(struct_name))
    if len(all_used_structs) > 0:
        cpp_scan_padding.write("        const auto &l = pointer.has_value() ? tag.get_struct_at_pointer<HEK::{}>(*pointer) : tag.get_base_struct<HEK::{}>();\n".format(struct_name, struct_name))
        cpp_scan_padding.write("        auto l_copy = l;\n")
        for struct in all_used_structs:
            name = struct["member_name"]
            if struct["type"] == "TagReflexive":
                cpp_scan_padding.write("        std::size_t l_{}_count = l.{}.count.read();\n".format(name, name))
                cpp_scan_padding.write("        if(l_{}_count > 0) {{\n".format(name))
                if "zero_on_index" in struct and struct["zero_on_index"]:
                    cpp_scan_padding.write("            auto l_{}_ptr = tag.is_indexed() ? 0 : l.{}.pointer.read();\n".format(name, name))
                else:
                    cpp_scan_padding.write("            auto l_{}_ptr = l.{}.pointer;\n".format(name, name))
                cpp_scan_padding.write("            for(std::size_t i = 0; i < l_{}_count; i++) {{\n".format(name))
                cpp_scan_padding.write("                {}::scan_padding(tag, l_{}_ptr + i * sizeof({}::struct_little));\n".format(struct["struct"], name, struct["struct"]))
                cpp_scan_padding.write("            }\n")
                cpp_scan_padding.write("        }\n")
            cpp_scan_padding.write("        std::memset(reinterpret_cast<void *>(&l_copy.{}), 0, sizeof(l_copy.{}));\n".format(name, name))
        cpp_scan_padding.write("        for(std::size_t i = 0; i < sizeof(l_copy); i++) {\n")
        cpp_scan_padding.write("            auto v = reinterpret_cast<const std::uint8_t *>(&l_copy)[i];\n")
        cpp_scan_padding.write("            if(v != 0) {\n")
        cpp_scan_padding.write("                oprintf(\"%s.%s: {} @ 0x%04zX - %02X\\n\", tag.get_path().c_str(), Invader::HEK::tag_class_to_extension(tag.get_tag_class_int()), i, v);\n".format(struct_name))
        cpp_scan_padding.write("            }\n")
        cpp_scan_padding.write("        }\n")
    cpp_scan_padding.write("    }\n")
