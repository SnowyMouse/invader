# SPDX-License-Identifier: GPL-3.0-only

def make_parse_hek_tag_data(definition, all_types, append_line):
    for s in definition["definitions"]:
        if s["type"] != "struct":
            continue
        struct_name = s["name"]
        append_line("{} {}::parse_hek_tag_data(const std::byte *data, std::size_t data_size, std::size_t &data_read, [[maybe_unused]] bool postprocess, const C<BigEndian> *data_this) {{".format(struct_name, struct_name))
        append_line("{} r = {{}};".format(struct_name), 1)
        
        # Set the data_read value to 0
        append_line("data_read = 0;", 1)
        
        # If the data_this field isn't set, set it to the data. This is where the big endian struct is
        append_line("if(data_this == nullptr) {", 1)
        append_line("if(sizeof(C<BigEndian>) > data_size) {", 2)
        append_line("eprintf_error(\"Failed to read {} base struct: %zu bytes needed > %zu bytes available\", sizeof(C<BigEndian>), data_size);".format(struct_name), 3)
        append_line("throw OutOfBoundsException();", 3)
        append_line("}", 2)
        append_line("data_this = reinterpret_cast<const C<BigEndian> *>(data);", 2)
        append_line("data_size -= sizeof(C<BigEndian>);", 2)
        append_line("data_read += sizeof(C<BigEndian>);", 2)
        append_line("data += sizeof(C<BigEndian>);", 2)
        append_line("}", 1)
        
        # Here's our big endian struct
        append_line("[[maybe_unused]] const auto &h = *data_this;", 1)
        
        # If it inherits anything, we should read that first
        if "inherits" in s:
            append_line("std::size_t data_read_super;", 1)
            append_line("dynamic_cast<{} &>(r) = {}::parse_hek_tag_data(data, data_size, data_read_super, false, &h);".format(s["inherits"], s["inherits"]), 1)
            append_line("data_read += data_read_super;", 1)
            append_line("data += data_read_super;", 1)
            append_line("data_size -= data_read_super;", 1)
        
        for field in s["fields"]:
            if field["type"] == "pad":
                continue
            name = field["member_name"]
            unread = ("cache_only" in field and field["cache_only"]) or ("unused" in field and field["unused"])
            if unread and field["type"] != "TagReflexive" and field["type"] != "TagDependency" and field["type"] != "TagDataOffset":
                continue
            default_sign = "<=" if "default_sign" in field and field["default_sign"] else "=="
            if field["type"] == "TagDependency":
                append_line("std::size_t h_{}_expected_length = h.{}.path_size;".format(name,name), 1)
                append_line("r.{}.tag_fourcc = h.{}.tag_fourcc;".format(name, name), 1)
                append_line("if(h_{}_expected_length > 0) {{".format(name), 1)
                append_line("if(h_{}_expected_length + 1 > data_size) {{".format(name), 2)
                append_line("eprintf_error(\"Failed to read dependency {}::{}: %zu bytes needed > %zu bytes available\", h_{}_expected_length, data_size);".format(struct_name, name, name), 3)
                append_line("throw OutOfBoundsException();", 3)
                append_line("}", 2)
                append_line("const char *h_{}_char = reinterpret_cast<const char *>(data);".format(name), 2)
                append_line("for(std::size_t i = 0; i < h_{}_expected_length; i++) {{".format(name), 2)
                append_line("if(h_{}_char[i] == 0) {{".format(name), 3)
                append_line("eprintf_error(\"Failed to read dependency {}::{}: size is smaller than expected (%zu expected > %zu actual)\", h_{}_expected_length, i);".format(struct_name, name, name), 4)
                append_line("throw InvalidTagDataException();", 4)
                append_line("}", 3)
                append_line("}", 2)
                append_line("if(static_cast<char>(data[h_{}_expected_length]) != 0) {{".format(name), 2)
                append_line("eprintf_error(\"Failed to read dependency {}::{}: missing null terminator\");".format(struct_name, name), 3)
                append_line("throw InvalidTagDataException();", 3)
                append_line("}", 2)
                if not unread:
                    append_line("r.{}.path = Invader::File::remove_duplicate_slashes(std::string(reinterpret_cast<const char *>(data)));".format(name), 2)
                append_line("data_size -= h_{}_expected_length + 1;".format(name), 2)
                append_line("data_read += h_{}_expected_length + 1;".format(name), 2)
                append_line("data += h_{}_expected_length + 1;".format(name), 2)
                append_line("}", 1)
                if field["classes"][0] != "*":
                    append_line("else if(r.{}.tag_fourcc == TagFourCC::TAG_FOURCC_NULL) {{".format(name), 1)
                    append_line("r.{}.tag_fourcc = TagFourCC::TAG_FOURCC_{};".format(name, field["classes"][0].upper()), 2)
                    append_line("}", 1)
            elif field["type"] == "TagReflexive":
                append_line("std::size_t h_{}_count = h.{}.count;".format(name,name), 1)
                append_line("if(h_{}_count > 0) {{".format(name), 1)
                append_line("const auto *array = reinterpret_cast<const C<BigEndian> *>(data);", 2)
                append_line("std::size_t total_size = sizeof(*array) * h_{}_count;".format(name), 2)
                append_line("if(total_size > data_size) {", 2)
                append_line("eprintf_error(\"Failed to read reflexive {}::{}: %zu bytes needed > %zu bytes available\", total_size, data_size);".format(struct_name, name), 3)
                append_line("throw OutOfBoundsException();", 3)
                append_line("}", 2)
                append_line("data_size -= total_size;", 2)
                append_line("data_read += total_size;", 2)
                append_line("data += total_size;", 2)
                if not unread:
                    append_line("r.{}.reserve(h_{}_count);".format(name, name), 2)
                append_line("for(std::size_t ref = 0; ref < h_{}_count; ref++) {{".format(name), 2)
                append_line("std::size_t ref_data_read = 0;", 3)
                call = "{}::parse_hek_tag_data(data, data_size, ref_data_read, postprocess, reinterpret_cast<const {}::C<BigEndian> *>(array + ref))".format(field["struct"], field["struct"])
                if not unread:
                    append_line("r.{}.emplace_back({});".format(name, call), 3)
                else:
                    append_line("{};".format(call), 3)
                append_line("data += ref_data_read;", 3)
                append_line("data_read += ref_data_read;", 3)
                append_line("data_size -= ref_data_read;", 3)
                append_line("}", 2)
                append_line("}", 1)
            elif field["type"] == "TagDataOffset":
                append_line("std::size_t h_{}_size = h.{}.size;".format(name, name), 1)
                append_line("if(h_{}_size > data_size) {{".format(name), 1)
                append_line("eprintf_error(\"Failed to read tag data block {}::{}: %zu bytes needed > %zu bytes available\", h_{}_size, data_size);".format(struct_name, name, name), 2)
                append_line("throw OutOfBoundsException();", 2)
                append_line("}", 1)
                if not unread:
                    append_line("r.{} = std::vector<std::byte>(data, data + h_{}_size);".format(name, name), 1)
                append_line("data_size -= h_{}_size;".format(name), 1)
                append_line("data_read += h_{}_size;".format(name), 1)
                append_line("data += h_{}_size;".format(name), 1)
            elif field["type"] == "ColorRGB":
                append_line("r.{} = h.{};".format(name, name), 1)
                if "default" in field:
                    default = field["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    append_line("if(postprocess && r.{}.red {} 0 && r.{}.green {} 0 && r.{}.blue {} 0) {{".format(name,default_sign,name,default_sign,name,default_sign), 1)
                    append_line("r.{}.red = {}{};".format(name, default[0], suffix), 2)
                    append_line("r.{}.green = {}{};".format(name, default[1], suffix), 2)
                    append_line("r.{}.blue = {}{};".format(name, default[2], suffix), 2)
                    append_line("}", 1)
            elif field["type"] == "ColorARGB" or field["type"] == "ColorARGBInt":
                append_line("r.{} = h.{};".format(name, name), 1)
                if "default" in field:
                    default = field["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    append_line("if(postprocess && r.{}.alpha {} 0 && r.{}.red {} 0 && r.{}.green {} 0 && r.{}.blue {} 0) {{".format(name,default_sign,name,default_sign,name,default_sign,name,default_sign), 1)
                    append_line("r.{}.alpha = {}{};".format(name, default[0], suffix), 2)
                    append_line("r.{}.red = {}{};".format(name, default[1], suffix), 2)
                    append_line("r.{}.green = {}{};".format(name, default[2], suffix), 2)
                    append_line("r.{}.blue = {}{};".format(name, default[3], suffix), 2)
                    append_line("}", 1)
            elif field["type"] == "TagID":
                append_line("r.{} = TagID::null_tag_id();".format(name), 1)
            elif "bounds" in field and field["bounds"]:
                append_line("r.{}.from = h.{}.from;".format(name, name), 1)
                append_line("r.{}.to = h.{}.to;".format(name, name), 1)
                if "default" in field:
                    default = field["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    append_line("if(postprocess && r.{}.from {} 0 && r.{}.to {} 0) {{".format(name, default_sign, name, default_sign), 1)
                    append_line("r.{}.from = {}{};".format(name, default[0], suffix), 2)
                    append_line("r.{}.to = {}{};".format(name, default[1], suffix), 2)
                    append_line("}", 1)
            elif "count" in field and field["count"] > 1:
                append_line("std::copy(h.{}, h.{} + {}, r.{});".format(name, name, field["count"], name), 1)
                if "default" in field:
                    default = field["default"]
                    suffix = "F" if isinstance(default[0], float) else ""
                    for q in range(field["count"]):
                        append_line("if(postprocess && r.{}[{}] {} 0) {{".format(name, q, default_sign), 1)
                        append_line("r.{}[{}] = {}{};".format(name, q, default[q], suffix), 2)
                        append_line("}", 1)
            else:
                # Handle bitfields
                if field["type"] in all_types and all_types[field["type"]]["type"] == "bitfield":
                    b = all_types[field["type"]]
                    added = True
                    mask = "~static_cast<std::uint{}_t>(0x{:X}) & static_cast<std::uint{}_t>(0x{:X})".format(b["width"], b["cache_only_mask"], b["width"], b["mask"])
                    append_line("r.{} = static_cast<std::uint{}_t>(h.{}) & {};".format(name, b["width"], name, mask), 1)
                else:
                    append_line("r.{} = h.{};".format(name, name), 1)
                    if "default" in field:
                        default = field["default"]
                        suffix = "F" if isinstance(default, float) else ""
                        append_line("if(postprocess && r.{} {} 0) {{".format(name, default_sign), 1)
                        append_line("r.{} = {}{};".format(name, default, suffix), 2)
                        append_line("}", 1)
        if "postprocess_hek_data" in s and s["postprocess_hek_data"]:
            append_line("if(postprocess) {", 1)
            append_line("r.postprocess_hek_data();", 2)
            append_line("}", 1)
        append_line("return r;", 1)
        append_line("}")
