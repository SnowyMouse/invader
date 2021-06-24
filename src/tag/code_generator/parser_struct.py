# SPDX-License-Identifier: GPL-3.0-only

def make_parser_struct(cpp_struct_value, all_enums, all_bitfields, all_used_structs, all_used_groups, hpp, struct_name, read_only, struct_title):
    hpp.write("    private:\n".format(struct_name))
    hpp.write("        std::vector<ParserStructValue> get_values_internal() override;\n".format(struct_name))
    hpp.write("    public:\n".format(struct_name))
    cpp_struct_value.write("std::vector<ParserStructValue> {}::get_values_internal() {{\n".format(struct_name))
    cpp_struct_value.write("    std::vector<ParserStructValue> values;\n")
    cpp_struct_value.write("    values.reserve({});\n".format(len(all_used_structs)))

    for struct in all_used_structs:
        if "hidden" in struct and struct["hidden"]:
            continue

        if ("cache_only" in struct and struct["cache_only"]) or ("endian" in struct and struct["endian"] == "little") or ("unused" in struct and struct["unused"]):
            continue
        
        def make_cpp_string(what):
            return "\"{}\"".format(what.replace("\"", "\\\"").replace("\n", "\\n"))

        name = "\"{}\"".format(struct["display_name"])
        member_name = struct["member_name"]
        member_name_q = "\"{}\"".format(member_name)
        comment = "nullptr" if "comment" not in struct else make_cpp_string(struct["comment"])
        struct_read_only = "true" if ((read_only or ("read_only" in struct and struct["read_only"])) and not ("read_only" in struct and struct["read_only"] == False)) else "false"
        unit = "nullptr" if "unit" not in struct else "\"{}\"".format(struct["unit"].replace("\"", "\\\""))
        
        # If this is the start of a group, add a group
        for i in all_used_groups:
            if i["first"] == struct["name"]:
                cpp_struct_value.write("    values.emplace_back(\"{}\", {});\n".format(i["name"], make_cpp_string(i["description"])))
                break

        first_arguments = "{},{},{},&this->{}".format(name, member_name_q, comment, struct["member_name"])
        type = struct["type"]
        
        if type == "TagDependency":
            classes = struct["classes"]
            classes_len = len(classes)

            if classes[0] == "*":
                cpp_struct_value.write("    values.emplace_back({}, nullptr, 0, {});\n".format(first_arguments, struct_read_only))
            else:
                cpp_struct_value.write("    TagFourCC {}_types[] = {{".format(member_name));
                for c in range(0, classes_len):
                    if c != 0:
                        cpp_struct_value.write(", ")
                    cpp_struct_value.write("TagFourCC::TAG_FOURCC_{}".format(classes[c].upper()))
                cpp_struct_value.write("};\n");
                cpp_struct_value.write("    values.emplace_back({}, {}_types, {}, {});\n".format(first_arguments, member_name, classes_len, struct_read_only))
        elif type == "TagReflexive":
            minimum = 0 if not ("minimum" in struct) else struct["minimum"]
            maximum = 0xFFFFFFFF if not ("maximum" in struct) else struct["maximum"]

            vstruct = "std::vector<{}>".format(struct["struct"])
            cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::get_object_in_array_template<{}>, ParserStructValue::get_array_size_template<{}>, ParserStructValue::delete_objects_in_array_template<{}>, ParserStructValue::insert_object_in_array_template<{}>, ParserStructValue::duplicate_object_in_array_template<{}>, ParserStructValue::swap_object_in_array_template<{}>, static_cast<std::size_t>({}), static_cast<std::size_t>({}), {});\n".format(first_arguments, vstruct, vstruct, vstruct, vstruct, vstruct, vstruct, minimum, maximum, struct_read_only))
        elif type == "TagDataOffset" or type == "TagString":
            cpp_struct_value.write("    values.emplace_back({}, {});\n".format(first_arguments, struct_read_only))
        elif type == "ScenarioScriptNodeValue" or type == "ScenarioStructureBSPArrayVertex":
            pass
        else:
            found = False
            for b in all_bitfields:
                if type == b["name"]:
                    found = True
                    type = "{}Flag".format(type)
                    
                    # Base mask (all existing fields)
                    mask = 2**len(b["fields"]) - 1
                    
                    # Hide unused cache-only stuff
                    if "cache_only" in b:
                        mask_cache_only = 0
                        for a in b["cache_only"]:
                            for n in range(0, len(b["fields"])):
                                if b["fields"][n] == a:
                                    mask_cache_only = mask_cache_only | (1 << n)
                                    break
                            
                        mask = (~mask_cache_only) & mask
                    
                    # Hide unused bitmasks
                    if "__excluded" in struct and struct["__excluded"] is not None:
                        mask = (~struct["__excluded"]) & mask
                    
                    # If mask is 0, disregard
                    if mask != 0:
                        break
                        
                    cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::list_bitmask_template<HEK::{}, HEK::{}_to_string, {}, 0x{:X}>, ParserStructValue::list_bitmask_template<HEK::{}, HEK::{}_to_string_pretty, {}, 0x{:X}>, ParserStructValue::read_bitfield_template<HEK::{}, HEK::{}_from_string>, ParserStructValue::write_bitfield_template<HEK::{}, HEK::{}_from_string>, {});\n".format(first_arguments, type, type, len(b["fields_formatted"]), mask, type, type, len(b["fields_formatted"]), mask, type, type, type, type, struct_read_only))
                    break
            if found:
                continue
            for e in all_enums:
                if type == e["name"]:
                    found = True
                    cpp_struct_value.write("    {\n")
                    ignorelist_params = ""
                    
                    # Make an ignorelist to hold stuff we don't want to list
                    if "__excluded" in struct and struct["__excluded"] is not None:
                        cpp_struct_value.write("    static HEK::{} ignorelist[] = {{\n".format(e["name"]))
                        for x in struct["__excluded"]:
                            cpp_struct_value.write("        static_cast<HEK::{}>({}),\n".format(e["name"], x))
                        cpp_struct_value.write("    };\n")
                        ignorelist_params = ", ignorelist, {}".format(len(struct["__excluded"]))
                        
                    # Do it!
                    list_enum_invocation = "ParserStructValue::list_enum_template<HEK::{}, HEK::{}_to_string{{}}, {}{}>".format(type, type, len(e["options_formatted"]), ignorelist_params)
                    
                    cpp_struct_value.write("        values.emplace_back({}, {}, {}, ParserStructValue::read_enum_template<HEK::{}, HEK::{}_to_string>, ParserStructValue::write_enum_template<HEK::{}, HEK::{}_from_string>, {});\n".format(first_arguments, list_enum_invocation.format(""), list_enum_invocation.format("_pretty"), type, type, type, type, struct_read_only))
                    cpp_struct_value.write("    }\n")
                    break
            if found:
                continue

            bounds_b = "bounds" in struct and struct["bounds"]
            bounds = "true" if bounds_b else "false"
            count = 1 * (2 if bounds_b else 1)
            minimum = "static_cast<ParserStructValue::Number>({})".format(struct["minimum"]) if "minimum" in struct else "std::nullopt"
            maximum = "static_cast<ParserStructValue::Number>({})".format(struct["maximum"]) if "maximum" in struct else "std::nullopt"
            volatile = "true" if ("volatile" in struct and struct["volatile"]) else "false"

            cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::ValueType::VALUE_TYPE_{}, {}, {}, {}, {}, {}, {}, {});\n".format(first_arguments, type.upper(), unit, count, bounds, volatile, struct_read_only, minimum, maximum))

    cpp_struct_value.write("    return values;\n")
    cpp_struct_value.write("}\n")
    
    hpp.write("        const char *struct_name() const override;\n")
    cpp_struct_value.write("const char *{}::struct_name() const {{\n".format(struct_name))
    cpp_struct_value.write("    return \"{}\";\n".format(struct_name))
    cpp_struct_value.write("}\n")

    if struct_title is not None:
        hpp.write("        bool has_title() const override;\n")
        cpp_struct_value.write("bool {}::has_title() const {{\n".format(struct_name))
        cpp_struct_value.write("    return true;\n")
        cpp_struct_value.write("}\n")
        hpp.write("        const char *title() const override;\n")
        cpp_struct_value.write("const char *{}::title() const {{\n".format(struct_name))
        for struct in all_used_structs:
            if "name" in struct and struct["name"] == struct_title:
                if struct["type"] == "TagString":
                    cpp_struct_value.write("    return this->{}.string;\n".format(struct["member_name"]))
                elif struct["type"] == "TagDependency":
                    cpp_struct_value.write("    const auto *start = this->{}.path.c_str();\n".format(struct["member_name"]))
                    cpp_struct_value.write("    for(const char *q = start; q && *q; q++) {\n")
                    cpp_struct_value.write("        if(*q == '\\\\') {\n")
                    cpp_struct_value.write("            start = q + 1;\n")
                    cpp_struct_value.write("        }\n")
                    cpp_struct_value.write("    }\n")
                    cpp_struct_value.write("    return start;\n")
                else:
                    raise Exception("ohno")
        cpp_struct_value.write("}\n")
