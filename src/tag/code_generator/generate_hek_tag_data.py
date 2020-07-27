# SPDX-License-Identifier: GPL-3.0-only

def make_cpp_save_hek_data(extract_hidden, all_bitfields, all_used_structs, struct_name, hpp, cpp_save_hek_data):
    hpp.write("        std::vector<std::byte> generate_hek_tag_data(std::optional<TagClassInt> generate_header_class = std::nullopt, bool clear_on_save = false) override;\n")
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
            if (("cache_only" in struct and struct["cache_only"]) or ("unused" in struct and struct["unused"])) and not extract_hidden:
                continue
            name = struct["member_name"]
            if "drop_on_extract_hidden" in struct and struct["drop_on_extract_hidden"]:
                cpp_save_hek_data.write("        b.{} = {{}};\n".format(name))
                continue
            if struct["type"] == "TagDependency":
                cpp_save_hek_data.write("        std::size_t {}_size = static_cast<std::uint32_t>(this->{}.path.size());\n".format(name,name))
                
                cpp_save_hek_data.write("        b.{}.tag_id = HEK::TagID::null_tag_id();\n".format(name))
                cpp_save_hek_data.write("        b.{}.tag_class_int = this->{}.tag_class_int;\n".format(name, name))
                cpp_save_hek_data.write("        if({}_size > 0) {{\n".format(name))
                cpp_save_hek_data.write("            b.{}.path_size = static_cast<std::uint32_t>({}_size);\n".format(name, name))
                cpp_save_hek_data.write("            const auto *path_str = reinterpret_cast<const std::byte *>(this->{}.path.c_str());\n".format(name))
                cpp_save_hek_data.write("            converted_data.insert(converted_data.end(), path_str, path_str + {}_size + 1);\n".format(name))
                cpp_save_hek_data.write("            if(clear_on_save) {\n")
                cpp_save_hek_data.write("                this->{}.path = std::string();\n".format(name))
                cpp_save_hek_data.write("            }\n")
                cpp_save_hek_data.write("        }\n")
                if struct["classes"][0] != "*":
                    cpp_save_hek_data.write("        else if(this->{}.tag_class_int == HEK::TagClassInt::TAG_CLASS_NULL) {{\n".format(name))
                    cpp_save_hek_data.write("            b.{}.tag_class_int = HEK::TagClassInt::TAG_CLASS_{};\n".format(name, struct["classes"][0].upper()))
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
            elif extract_hidden and struct["type"] == "float" and "bounds" not in struct and "count" not in struct:
                cpp_save_hek_data.write("        union {{ float f; std::uint32_t i; }} {}_discarded;\n".format(name))
                cpp_save_hek_data.write("        {}_discarded.f = static_cast<std::int32_t>(this->{} * 1000.0F) / 1000.0F;\n".format(name, name)) # first, round to the nearest 0.0001 for rounding errors
                cpp_save_hek_data.write("        {}_discarded.i &= 0xFFFFFF00;\n".format(name)) # next, discard the last eight bits for rounding errors
                cpp_save_hek_data.write("        if({}_discarded.i == 0x80000000) {{ {}_discarded.i = 0; }}\n".format(name, name)) # last, if negative 0, set to 0
                cpp_save_hek_data.write("        b.{} = {}_discarded.f;\n".format(name, name))
            elif "bounds" in struct and struct["bounds"]:
                cpp_save_hek_data.write("        b.{}.from = this->{}.from;\n".format(name, name))
                cpp_save_hek_data.write("        b.{}.to = this->{}.to;\n".format(name, name))
            elif "count" in struct and struct["count"] > 1:
                cpp_save_hek_data.write("        std::copy(this->{}, this->{} + {}, b.{});\n".format(name, name, struct["count"], name))
            else:
                negate = ""
                for b in all_bitfields:
                    if b["name"] == struct["type"]:
                        if "cache_only" in b:
                            added = True
                            for c in b["cache_only"]:
                                for i in range(0,len(b["fields"])):
                                    if b["fields"][i] == c:
                                        negate = "{} & ~static_cast<std::uint{}_t>(0x{:X})".format(negate, b["width"], 1 << i)
                                        break
                        if "__excluded" in struct:
                            negate = "{} & ~static_cast<std::uint{}_t>(0x{:X})".format(negate, b["width"], struct["__excluded"])
                cpp_save_hek_data.write("        b.{} = this->{}{};\n".format(name, name, negate))
        cpp_save_hek_data.write("        *reinterpret_cast<struct_big *>(converted_data.data() + tag_header_offset) = b;\n")
    cpp_save_hek_data.write("        if(generate_header_class.has_value()) {\n")
    cpp_save_hek_data.write("            reinterpret_cast<HEK::TagFileHeader *>(converted_data.data())->crc32 = ~crc32(clear_on_save ^ clear_on_save, reinterpret_cast<const void *>(converted_data.data() + tag_header_offset), converted_data.size() - tag_header_offset);\n")
    cpp_save_hek_data.write("        }\n")
    cpp_save_hek_data.write("        return converted_data;\n")
    cpp_save_hek_data.write("    }\n")
