# SPDX-License-Identifier: GPL-3.0-only

def make_parse_hek_tag_data(postprocess_hek_data, all_bitfields, struct_name, all_used_structs, hpp, cpp_read_hek_data):
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
        cpp_read_hek_data.write("        [[maybe_unused]] const auto &h = *reinterpret_cast<const HEK::{}<HEK::BigEndian> *>(data_this);\n".format(struct_name))
        for struct in all_used_structs:
            name = struct["member_name"]
            cache_only = "cache_only" in struct and struct["cache_only"]
            if cache_only and struct["type"] != "TagReflexive":
                continue
            default_sign = "<=" if "default_sign" in struct and struct["default_sign"] else "=="
            if struct["type"] == "TagDependency":
                cpp_read_hek_data.write("        std::size_t h_{}_expected_length = h.{}.path_size;\n".format(name,name))
                
                cpp_read_hek_data.write("        r.{}.tag_class_int = h.{}.tag_class_int;\n".format(name, name))
                cpp_read_hek_data.write("        if(h_{}_expected_length > 0) {{\n".format(name))
                cpp_read_hek_data.write("            if(h_{}_expected_length + 1 > data_size) {{\n".format(name))
                cpp_read_hek_data.write("                eprintf_error(\"Failed to read dependency {}::{}: %zu bytes needed > %zu bytes available\", h_{}_expected_length, data_size);\n".format(struct_name, name, name))
                cpp_read_hek_data.write("                throw OutOfBoundsException();\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            const char *h_{}_char = reinterpret_cast<const char *>(data);\n".format(name))
                cpp_read_hek_data.write("            for(std::size_t i = 0; i < h_{}_expected_length; i++) {{\n".format(name))
                cpp_read_hek_data.write("                if(h_{}_char[i] == 0) {{\n".format(name))
                cpp_read_hek_data.write("                    eprintf_error(\"Failed to read dependency {}::{}: size is smaller than expected (%zu expected > %zu actual)\", h_{}_expected_length, i);\n".format(struct_name, name, name))
                cpp_read_hek_data.write("                    throw InvalidTagDataException();\n")
                cpp_read_hek_data.write("                }\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            if(static_cast<char>(data[h_{}_expected_length]) != 0) {{\n".format(name))
                cpp_read_hek_data.write("                eprintf_error(\"Failed to read dependency {}::{}: missing null terminator\");\n".format(struct_name, name))
                cpp_read_hek_data.write("                throw InvalidTagDataException();\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            r.{}.path = Invader::File::remove_duplicate_slashes(std::string(reinterpret_cast<const char *>(data)));\n".format(name))
                cpp_read_hek_data.write("            data_size -= h_{}_expected_length + 1;\n".format(name))
                cpp_read_hek_data.write("            data_read += h_{}_expected_length + 1;\n".format(name))
                cpp_read_hek_data.write("            data += h_{}_expected_length + 1;\n".format(name))
                cpp_read_hek_data.write("        }\n")
                if struct["classes"][0] != "*":
                    cpp_read_hek_data.write("        else if(r.{}.tag_class_int == HEK::TagClassInt::TAG_CLASS_NULL) {{\n".format(name))
                    cpp_read_hek_data.write("            r.{}.tag_class_int = HEK::TagClassInt::TAG_CLASS_{};\n".format(name, struct["classes"][0].upper()))
                    cpp_read_hek_data.write("        }\n")
            elif struct["type"] == "TagReflexive":
                cpp_read_hek_data.write("        std::size_t h_{}_count = h.{}.count;\n".format(name,name))
                cpp_read_hek_data.write("        if(h_{}_count > 0) {{\n".format(name))
                cpp_read_hek_data.write("            const auto *array = reinterpret_cast<const HEK::{}<HEK::BigEndian> *>(data);\n".format(struct["struct"]))
                cpp_read_hek_data.write("            std::size_t total_size = sizeof(*array) * h_{}_count;\n".format(name))
                cpp_read_hek_data.write("            if(total_size > data_size) {\n")
                cpp_read_hek_data.write("                eprintf_error(\"Failed to read reflexive {}::{}: %zu bytes needed > %zu bytes available\", total_size, data_size);\n".format(struct_name, name))
                cpp_read_hek_data.write("                throw OutOfBoundsException();\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("            data_size -= total_size;\n")
                cpp_read_hek_data.write("            data_read += total_size;\n")
                cpp_read_hek_data.write("            data += total_size;\n")
                if not cache_only:
                    cpp_read_hek_data.write("            r.{}.reserve(h_{}_count);\n".format(name, name))
                cpp_read_hek_data.write("            for(std::size_t ref = 0; ref < h_{}_count; ref++) {{\n".format(name))
                cpp_read_hek_data.write("                std::size_t ref_data_read = 0;\n")
                call = "{}::parse_hek_tag_data(data, data_size, ref_data_read, postprocess, reinterpret_cast<const std::byte *>(array + ref))".format(struct["struct"])
                if not cache_only:
                    cpp_read_hek_data.write("                r.{}.emplace_back({});\n".format(name, call))
                else:
                    cpp_read_hek_data.write("                {};\n".format(call))
                cpp_read_hek_data.write("                data += ref_data_read;\n")
                cpp_read_hek_data.write("                data_read += ref_data_read;\n")
                cpp_read_hek_data.write("                data_size -= ref_data_read;\n")
                cpp_read_hek_data.write("            }\n")
                cpp_read_hek_data.write("        }\n")
            elif struct["type"] == "TagDataOffset":
                cpp_read_hek_data.write("        std::size_t h_{}_size = h.{}.size;\n".format(name, name))
                cpp_read_hek_data.write("        if(h_{}_size > data_size) {{\n".format(name))
                cpp_read_hek_data.write("            eprintf_error(\"Failed to read tag data block {}::{}: %zu bytes needed > %zu bytes available\", h_{}_size, data_size);\n".format(struct_name, name, name))
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
                added = False
                for b in all_bitfields:
                    if b["name"] == struct["type"]:
                        added = True
                        cpp_read_hek_data.write("        r.{} = static_cast<std::uint{}_t>(h.{}) & static_cast<std::uint{}_t>(0x{:X});\n".format(name, b["width"], name, b["width"], (1 << len(b["fields"])) - 1))
                        break
                if not added:
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
