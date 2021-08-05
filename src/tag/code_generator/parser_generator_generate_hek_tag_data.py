# SPDX-License-Identifier: GPL-3.0-only

def make_generate_hek_tag_data(definition, append_line):
    for d in definition["definitions"]:
        if d["type"] == "struct":
            append_line("std::vector<std::byte> {}::generate_hek_tag_data([[maybe_unused]] bool clear_on_save) {{".format(struct_name))
            append_line("    this->cache_deformat();")
            append_line("    std::vector<std::byte> converted_data(sizeof(struct_big));")
            append_line("    std::size_t tag_header_offset = 0;")
            if "group" in d:
                append_line("    HEK::TagFileHeader header(TagFourCC::TAG_FOURCC_{});".format(d["group"].upper()))
                append_line("    tag_header_offset = sizeof(header);")
                append_line("    converted_data.insert(converted_data.begin(), reinterpret_cast<std::byte *>(&header), reinterpret_cast<std::byte *>(&header + 1));")
            if len(all_used_structs) > 0:
                append_line("    struct_big b = {};")
                for struct in all_used_structs:
                    if (("cache_only" in struct and struct["cache_only"]) or ("unused" in struct and struct["unused"])):
                        continue
                    name = struct["member_name"]
                    if "drop_on_extract_hidden" in struct and struct["drop_on_extract_hidden"]:
                        append_line("    b.{} = {{}};".format(name), 1)
                        continue
                    if struct["type"] == "TagDependency":
                        append_line("    std::size_t {}_size = static_cast<std::uint32_t>(this->{}.path.size());".format(name,name), 1)
                        append_line("    b.{}.tag_id = HEK::TagID::null_tag_id();".format(name), 1)
                        append_line("    b.{}.tag_fourcc = this->{}.tag_fourcc;".format(name, name), 1)
                        append_line("    if({}_size > 0) {{".format(name), 1)
                        append_line("        b.{}.path_size = static_cast<std::uint32_t>({}_size);".format(name, name), 2)
                        append_line("        const auto *path_str = reinterpret_cast<const std::byte *>(this->{}.path.c_str());".format(name), 2)
                        append_line("        converted_data.insert(converted_data.end(), path_str, path_str + {}_size + 1);".format(name), 2)
                        append_line("        if(clear_on_save) {", 2)
                        append_line("            this->{}.path = std::string();".format(name), 3)
                        append_line("        }", 2)
                        append_line("    }", 1)
                        if struct["classes"][0] != "*":
                            append_line("    else if(this->{}.tag_fourcc == HEK::TagFourCC::TAG_FOURCC_NULL) {{".format(name), 1)
                            append_line("        b.{}.tag_fourcc = HEK::TagFourCC::TAG_FOURCC_{};".format(name, struct["classes"][0].upper()), 2)
                            append_line("    }", 1)
                            
                    elif struct["type"] == "TagReflexive":
                        append_line("    auto ref_{}_size = this->{}.size();".format(name, name), 1)
                        append_line("    if(ref_{}_size > 0) {{".format(name), 1)
                        append_line("        b.{}.count = static_cast<std::uint32_t>(ref_{}_size);".format(name, name), 2)
                        append_line("        constexpr std::size_t STRUCT_SIZE = sizeof({}::struct_big);".format(struct["struct"]), 2)
                        append_line("        auto total_size = STRUCT_SIZE * ref_{}_size;".format(name), 2)
                        append_line("        const std::size_t FIRST_STRUCT_OFFSET = converted_data.size();", 2)
                        append_line("        converted_data.insert(converted_data.end(), total_size, std::byte());", 2)
                        append_line("        for(std::size_t i = 0; i < ref_{}_size; i++) {{".format(name))
                        append_line("            const auto converted_struct = this->{}[i].generate_hek_tag_data(clear_on_save);".format(name), 3)
                        append_line("            const auto *struct_data = converted_struct.data();")
                        append_line("            std::copy(struct_data, struct_data + STRUCT_SIZE, converted_data.data() + FIRST_STRUCT_OFFSET + STRUCT_SIZE * i);", 3)
                        append_line("            converted_data.insert(converted_data.end(), struct_data + STRUCT_SIZE, struct_data + converted_struct.size());", 3)
                        append_line("        }", 2)
                        append_line("        if(clear_on_save) {", 2)
                        append_line("            this->{} = std::vector<{}>();".format(name, struct["struct"]), 3)
                        append_line("        }", 2)
                        append_line("    }", 1)
                    elif struct["type"] == "TagDataOffset":
                        append_line("    b.{}.size = static_cast<std::uint32_t>(this->{}.size());".format(name, name)), 1
                        append_line("    converted_data.insert(converted_data.end(), this->{}.begin(), this->{}.end());".format(name, name, name), 1)
                        append_line("    if(clear_on_save) {")
                        append_line("        this->{} = std::vector<std::byte>();".format(name), 2)
                        append_line("    }", 1)
                    elif "bounds" in struct and struct["bounds"]:
                        append_line("    b.{}.from = this->{}.from;".format(name, name), 1)
                        append_line("    b.{}.to = this->{}.to;".format(name, name), 1)
                    elif "count" in struct and struct["count"] > 1:
                        append_line("    std::copy(this->{}, this->{} + {}, b.{});".format(name, name, struct["count"], name), 1)
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
                                if "__excluded" in struct and struct["__excluded"] is not None:
                                    negate = "{} & ~static_cast<std::uint{}_t>(0x{:X})".format(negate, b["width"], struct["__excluded"])
                        append_line("    b.{} = this->{}{};".format(name, name, negate), 1)
                append_line("    *reinterpret_cast<struct_big *>(converted_data.data() + tag_header_offset) = b;", 1)
            append_line("    if(generate_header_class.has_value()) {", 1)
            append_line("        reinterpret_cast<HEK::TagFileHeader *>(converted_data.data())->crc32 = ~crc32(0, reinterpret_cast<const void *>(converted_data.data() + tag_header_offset), converted_data.size() - tag_header_offset);", 2)
            append_line("    }", 1)
            append_line("    return converted_data;", 1)
            append_line("}")
