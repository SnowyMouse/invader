# SPDX-License-Identifier: GPL-3.0-only

def make_parse_cache_file_data(definition, all_types, append_line):
    for d in definition["definitions"]:
        if d["type"] == "struct":
            struct_name = d["name"]
            
            # Let's begin
            append_line("{} {}::parse_cache_file_data([[maybe_unused]] const Invader::Tag &tag, [[maybe_unused]] std::optional<Pointer> pointer) {{".format(struct_name, struct_name))
            append_line("{} r = {{}};".format(struct_name), 1)

            # If we inherit anything, parse that too
            if "inherits" in d:
                append_line("dynamic_cast<{} &>(r).parse_cache_file_data(tag, pointer);".format(d["inherits"]), 1)
            
            append_line("r.cache_formatted = true;", 1)
            append_line("[[maybe_unused]] const auto &l = pointer.has_value() ? tag.get_struct_at_pointer<{}::C>(*pointer) : tag.get_base_struct<{}::C>();".format(struct_name, struct_name), 1)
            for field in d["fields"]:
                if field["type"] == "pad":
                    continue
                
                name = field["member_name"]
                if field["type"] == "TagID":
                    append_line("r.{} = TagID::null_tag_id();".format(name), 1)
                    continue
                if ("non_cached" in field and field["non_cached"]) or ("unused" in field and field["unused"]) or ("ignore_cached" in field and field["ignore_cached"]):
                    continue
                
                # Handle dependencies
                if field["type"] == "TagDependency":
                    append_line("r.{}.tag_fourcc = l.{}.tag_fourcc.read();".format(name, name), 1)
                    append_line("r.{}.tag_id = l.{}.tag_id.read();".format(name, name), 1)
                    append_line("if(!r.{}.tag_id.is_null()) {{".format(name), 1)
                    append_line("try {", 2)
                    append_line("auto &referenced_tag = tag.get_map().get_tag(r.{}.tag_id.index);".format(name), 3)
                    append_line("if(referenced_tag.get_tag_fourcc() != r.{}.tag_fourcc) {{".format(name), 3)
                    append_line("eprintf_error(\"Corrupt tag reference (class in reference does not match class in referenced tag)\");", 4)
                    append_line("throw InvalidTagDataException();", 4)
                    append_line("}", 3)
                    append_line("r.{}.path = referenced_tag.get_path();".format(name), 3)
                    append_line("}", 2)
                    append_line("catch (std::exception &) {", 2)
                    append_line("eprintf_error(\"Invalid reference for {}::{} in %s.%s\", halo_path_to_preferred_path(tag.get_path()).c_str(), tag_fourcc_to_extension(tag.get_tag_fourcc()));".format(struct_name, name), 3)
                    append_line("throw;", 3)
                    append_line("}", 2)
                    append_line("for(char &c : r.{}.path) {{".format(name), 2)
                    append_line("c = std::tolower(c);", 3)
                    append_line("}", 2)
                    append_line("}", 1)
                    if field["classes"][0] != "*":
                        append_line("else if(r.{}.tag_fourcc == TagFourCC::TAG_FOURCC_NULL) {{".format(name), 1)
                        append_line("r.{}.tag_fourcc = TagFourCC::TAG_FOURCC_{};".format(name, field["classes"][0].upper()), 2)
                        append_line("}", 1)
                
                # Arrays
                elif field["type"] == "TagReflexive":
                    append_line("std::size_t l_{}_count = l.{}.count.read();".format(name, name), 1)
                    append_line("r.{}.reserve(l_{}_count);".format(name, name), 1)
                    append_line("if(l_{}_count > 0) {{".format(name), 1)
                    if "zero_on_index" in field and field["zero_on_index"]:
                        append_line("auto l_{}_ptr = tag.is_indexed() ? 0 : l.{}.pointer.read();".format(name, name), 2)
                    else:
                        append_line("auto l_{}_ptr = l.{}.pointer;".format(name, name), 2)
                    append_line("for(std::size_t i = 0; i < l_{}_count; i++) {{".format(name), 2)
                    append_line("try {", 3)
                    append_line("r.{}.emplace_back({}::parse_cache_file_data(tag, l_{}_ptr + i * sizeof({}::C<LittleEndian>)));".format(name, field["struct"], name, field["struct"]), 4)
                    append_line("}", 3)
                    append_line("catch (std::exception &) {", 3)
                    append_line("eprintf_error(\"Failed to parse {}::{} #%zu in %s.%s\", i, halo_path_to_preferred_path(tag.get_path()).c_str(), tag_fourcc_to_extension(tag.get_tag_fourcc()));".format(struct_name, name), 4)
                    append_line("throw;", 4)
                    append_line("}", 3)
                    append_line("}", 2)
                    append_line("}", 1)
                    
                # Data
                elif field["type"] == "TagDataOffset":
                    append_line("std::size_t l_{}_data_size = l.{}.size;".format(name, name), 1)
                    append_line("if(l_{}_data_size > 0) {{".format(name))
                    append_line("const std::byte *data;", 2)
                    append_line("try {", 2)
                    if "file_offset" in field:
                        if "external_file_offset" in field:
                            where_to = "DATA_MAP_CACHE"
                            if field["external_file_offset"] == "sounds.map":
                                where_to = "DATA_MAP_SOUND"
                            elif field["external_file_offset"] == "bitmaps.map":
                                where_to = "DATA_MAP_BITMAP"
                            elif field["external_file_offset"] == "loc.map":
                                where_to = "DATA_MAP_LOC"
                            else:
                                print("Unknown external_file_offset: {}".format(field["external_file_offset"]), file=sys.stderr)
                                sys.exit(1)

                            external_in_bitmap_or_sound = (field["external_file_offset"] == "bitmaps.map") or (field["external_file_offset"] == "sounds.map")

                            if external_in_bitmap_or_sound:
                                append_line("if(l.{}.external & 1) {{".format(name), 3)
                            append_line("{}data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size, Map::DataMapType::{});".format("" if external_in_bitmap_or_sound else "", name, name, where_to), 4)
                            if external_in_bitmap_or_sound:
                                append_line("}", 3)
                                append_line("else {", 3)
                                append_line("data = tag.get_map().get_internal_asset(l.{}.file_offset, l_{}_data_size);".format(name, name), 4)
                                append_line("}", 3)
                            pass
                        else:
                            append_line("data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size);".format(name, name), 3)
                        pass
                    else:
                        append_line("data = tag.data(l.{}.pointer, l_{}_data_size);".format(name, name), 3)
                    append_line("}", 2)
                    append_line("catch (std::exception &) {", 2)
                    append_line("eprintf_error(\"Failed to read tag data for {}::{} in %s.%s\", halo_path_to_preferred_path(tag.get_path()).c_str(), tag_fourcc_to_extension(tag.get_tag_fourcc()));".format(struct_name, name), 3)
                    append_line("throw;", 3)
                    append_line("}", 2)
                    append_line("r.{}.insert(r.{}.begin(), data, data + l_{}_data_size);".format(name, name, name), 2)
                    append_line("}", 1)
                    
                # These are fun
                elif "bounds" in field and field["bounds"]:
                    append_line("r.{}.from = l.{}.from;".format(name, name), 1)
                    append_line("r.{}.to = l.{}.to;".format(name, name), 1)
                    
                # More... arrays
                elif "count" in field and field["count"] > 1:
                    append_line("std::copy(l.{}, l.{} + {}, r.{});".format(name, name, field["count"], name), 1)
                    
                # Everything else
                else:
                    if field["type"] in all_types and all_types[field["type"]]["type"] == "bitfield":
                        b = all_types[field["type"]]
                        added = True
                        negate = ""
                        if "cache_only" in b:
                            added = True
                            negate = ""
                            for c in b["cache_only"]:
                                for i in range(0,len(b["fields"])):
                                    if b["fields"][i] == c:
                                        negate = "{} & ~static_cast<std::uint{}_t>(0x{:X})".format(negate, b["width"], 2 << i)
                                        break
                        append_line("r.{} = static_cast<std::uint{}_t>(l.{}) & static_cast<std::uint{}_t>(0x{:X}){};".format(name, b["width"], name, b["width"], (1 << len(b["fields"])) - 1, negate), 1)
                    else:
                        append_line("r.{} = l.{};".format(name, name), 1)
                        
            if "post_cache_parse" in d and d["post_cache_parse"]:
                append_line("r.post_cache_parse(tag, pointer);", 1)
            append_line("return r;", 1)
            append_line("}")
