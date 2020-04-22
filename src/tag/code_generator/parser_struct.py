# SPDX-License-Identifier: GPL-3.0-only

def make_parser_struct(cpp_struct_value, all_enums, all_bitfields, all_used_structs, hpp, struct_name, extract_hidden, read_only, struct_title):
    hpp.write("        std::vector<ParserStructValue> get_values() override;\n".format(struct_name))
    cpp_struct_value.write("std::vector<ParserStructValue> {}::get_values() {{\n".format(struct_name))
    cpp_struct_value.write("    std::vector<ParserStructValue> values;\n")
    
    if not extract_hidden:
        for struct in all_used_structs:
            if "hidden" in struct and struct["hidden"]:
                continue

            if ("cache_only" in struct and struct["cache_only"]) or ("endian" in struct and struct["endian"] == "little") or ("unused" in struct and struct["unused"]):
                continue

            name = "\"{}\"".format(struct["name"])
            member_name = struct["member_name"]
            member_name_q = "\"{}\"".format(member_name)
            comment = "nullptr" if "comment" not in struct else "\"{}\"".format(struct["comment"].replace("\"", "\\\"").replace("\n", "\\n\\n"))
            struct_read_only = "true" if ((read_only or ("read_only" in struct and struct["read_only"])) and not ("read_only" in struct and struct["read_only"] == False)) else "false"
            unit = "nullptr" if "unit" not in struct else "\"{}\"".format(struct["unit"].replace("\"", "\\\""))
            ref_struct = "nullptr" if "struct" not in struct else "\"{}\"".format(struct["struct"].replace("\"", "\\\""))
            reflexive = "nullptr" if "reflexive" not in struct else "\"{}\"".format(struct["reflexive"].replace("\"", "\\\""))

            first_arguments = "{},{},{},&this->{}".format(name, member_name_q, comment, struct["member_name"])
            type = struct["type"]

            if type == "TagDependency":
                classes = struct["classes"]
                classes_len = len(classes)

                if classes[0] == "*":
                    cpp_struct_value.write("    values.emplace_back({}, nullptr, 0, {});\n".format(first_arguments, struct_read_only))
                else:
                    cpp_struct_value.write("    TagClassInt {}_types[] = {{".format(member_name));
                    for c in range(0, classes_len):
                        if c != 0:
                            cpp_struct_value.write(", ")
                        cpp_struct_value.write("TagClassInt::TAG_CLASS_{}".format(classes[c].upper()))
                    cpp_struct_value.write("};\n");
                    cpp_struct_value.write("    values.emplace_back({}, {}_types, {}, {});\n".format(first_arguments, member_name, classes_len, struct_read_only))
            elif type == "TagReflexive":
                minimum = 0 if not ("minimum" in struct) else struct["minimum"]
                maximum = 0xFFFFFFFF if not ("maximum" in struct) else struct["maximum"]

                vstruct = "std::vector<{}>".format(struct["struct"])
                cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::get_object_in_array_template<{}>, ParserStructValue::get_array_size_template<{}>, ParserStructValue::delete_objects_in_array_template<{}>, ParserStructValue::insert_object_in_array_template<{}>, ParserStructValue::duplicate_object_in_array_template<{}>, static_cast<std::size_t>({}), static_cast<std::size_t>({}), {});\n".format(first_arguments, vstruct, vstruct, vstruct, vstruct, vstruct, minimum, maximum, struct_read_only))
            elif type == "TagDataOffset" or type == "TagString":
                cpp_struct_value.write("    values.emplace_back({}, {});\n".format(first_arguments, struct_read_only))
            elif type == "ScenarioScriptNodeValue" or type == "ScenarioStructureBSPArrayVertex":
                pass
            elif type == "Index":
                cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::ValueType::VALUE_TYPE_{}, {}, {}, 1, false, {}, {});\n".format(first_arguments, type.upper(), unit, struct_read_only, ref_struct, reflexive))
            else:
                found = False
                for b in all_bitfields:
                    if type == b["name"]:
                        found = True
                        type = "{}Flag".format(type)
                        
                        mask = 0xFFFFFFFF
                        
                        if "cache_only" in b:
                            mask = 0
                            for a in b["cache_only"]:
                                for n in range(0, len(b["fields"])):
                                    if b["fields"][n] == a:
                                        mask = mask | (1 << n)
                                        break
                                
                            mask = (~mask) & 0xFFFFFFFF
                        
                        cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::list_bitmask_template<HEK::{}, HEK::{}_to_string, {}, 0x{:X}>, ParserStructValue::list_bitmask_template<HEK::{}, HEK::{}_to_string_pretty, {}, 0x{:X}>, ParserStructValue::read_bitfield_template<HEK::{}, HEK::{}_from_string>, ParserStructValue::write_bitfield_template<HEK::{}, HEK::{}_from_string>, {});\n".format(first_arguments, type, type, len(b["fields_formatted"]), mask, type, type, len(b["fields_formatted"]), mask, type, type, type, type, struct_read_only))
                        break
                if found:
                    continue
                for e in all_enums:
                    if type == e["name"]:
                        found = True
                        cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::list_enum_template<HEK::{}, HEK::{}_to_string, {}>, ParserStructValue::list_enum_template<HEK::{}, HEK::{}_to_string_pretty, {}>, ParserStructValue::read_enum_template<HEK::{}, HEK::{}_to_string>, ParserStructValue::write_enum_template<HEK::{}, HEK::{}_from_string>, {});\n".format(first_arguments, type, type, len(e["options_formatted"]), type, type, len(e["options_formatted"]), type, type, type, type, struct_read_only))
                        break
                if found:
                    continue

                bounds_b = "bounds" in struct and struct["bounds"]
                bounds = "true" if bounds_b else "false"
                count = 1 * (2 if bounds_b else 1)
                minimum = "static_cast<ParserStructValue::Number>({})".format(struct["minimum"]) if "minimum" in struct else "std::nullopt"
                maximum = "static_cast<ParserStructValue::Number>({})".format(struct["maximum"]) if "maximum" in struct else "std::nullopt"

                cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::ValueType::VALUE_TYPE_{}, {}, {}, {}, {}, {}, {});\n".format(first_arguments, type.upper(), unit, count, bounds, struct_read_only, minimum, maximum))

    cpp_struct_value.write("    return values;\n")
    cpp_struct_value.write("}\n")
    
    hpp.write("        const char *struct_name() const override;\n")
    cpp_struct_value.write("const char *{}::struct_name() const {{\n".format(struct_name))
    cpp_struct_value.write("    return \"{}\";\n".format(struct_name))
    cpp_struct_value.write("}\n")

    if not struct_title is None:
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
                    raise "ohno"
        cpp_struct_value.write("}\n")
