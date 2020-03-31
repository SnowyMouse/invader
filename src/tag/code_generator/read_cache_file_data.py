# SPDX-License-Identifier: GPL-3.0-only

def make_parse_cache_file_data(post_cache_parse, all_used_structs, struct_name, hpp, cpp_read_cache_file_data):
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
            name = struct["member_name"]
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
                cpp_read_cache_file_data.write("                eprintf_error(\"Invalid reference for {}::{} in %s.%s\", File::halo_path_to_preferred_path(tag.get_path()).c_str(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
                cpp_read_cache_file_data.write("                throw;\n")
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("            for(char &c : r.{}.path) {{\n".format(name))
                cpp_read_cache_file_data.write("                c = std::tolower(c);\n")
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("        }\n")
                if struct["classes"][0] != "*":
                    cpp_read_cache_file_data.write("        else if(r.{}.tag_class_int == HEK::TagClassInt::TAG_CLASS_NULL) {{\n".format(name))
                    cpp_read_cache_file_data.write("            r.{}.tag_class_int = HEK::TagClassInt::TAG_CLASS_{};\n".format(name, struct["classes"][0].upper()))
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
                cpp_read_cache_file_data.write("                    eprintf_error(\"Failed to parse {}::{} #%zu in %s.%s\", i, File::halo_path_to_preferred_path(tag.get_path()).c_str(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
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

                        external_in_bitmap_or_sound = (struct["external_file_offset"] == "bitmaps.map") or (struct["external_file_offset"] == "sounds.map")

                        if external_in_bitmap_or_sound:
                            cpp_read_cache_file_data.write("                if(l.{}.external & 1) {{\n".format(name))
                        cpp_read_cache_file_data.write("                {}data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size, Map::DataMapType::{});".format("    " if external_in_bitmap_or_sound else "", name, name, where_to))
                        if external_in_bitmap_or_sound:
                            cpp_read_cache_file_data.write("                }")
                            cpp_read_cache_file_data.write("                else {")
                            cpp_read_cache_file_data.write("                    data = tag.get_map().get_internal_asset(l.{}.file_offset, l_{}_data_size);".format(name, name))
                            cpp_read_cache_file_data.write("                }")
                        pass
                    else:
                        cpp_read_cache_file_data.write("                data = tag.get_map().get_data_at_offset(l.{}.file_offset, l_{}_data_size);\n".format(name, name))
                    pass
                else:
                    cpp_read_cache_file_data.write("                data = tag.data(l.{}.pointer, l_{}_data_size);\n".format(name, name))
                cpp_read_cache_file_data.write("            }\n")
                cpp_read_cache_file_data.write("            catch (std::exception &) {\n")
                cpp_read_cache_file_data.write("                eprintf_error(\"Failed to read tag data for {}::{} in %s.%s\", File::preferred_path_to_halo_path(tag.get_path()).c_str(), HEK::tag_class_to_extension(tag.get_tag_class_int()));\n".format(struct_name, name))
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
