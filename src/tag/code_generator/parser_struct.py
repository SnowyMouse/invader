# SPDX-License-Identifier: GPL-3.0-only

def make_parser_struct(cpp_struct_value, all_enums, all_bitfields, all_used_structs, hpp, struct_name, extract_hidden):
    hpp.write("        std::vector<ParserStructValue> get_values() override;\n".format(struct_name))
    cpp_struct_value.write("std::vector<ParserStructValue> {}::get_values() {{\n".format(struct_name))
    cpp_struct_value.write("    std::vector<ParserStructValue> values;\n")
    for struct in all_used_structs:
        if "hidden" in struct and struct["hidden"]:
            continue

        if (("cache_only" in struct and struct["cache_only"]) or ("endian" in struct and struct["endian"] == "little")) and not extract_hidden:
            continue

        name = "\"{}\"".format(struct["name"])
        member_name = struct["member_name"]
        member_name_q = "\"{}\"".format(member_name)
        comment = "nullptr" if "comment" not in struct else "\"{}\"".format(struct["comment"])

        first_arguments = "{},{},{},&this->{}".format(name, member_name_q, comment, struct["member_name"])
        type = struct["type"]

        if type == "TagDependency":
            classes = struct["classes"]
            classes_len = len(classes)

            if classes[0] == "*":
                cpp_struct_value.write("    values.emplace_back({}, nullptr, 0);\n".format(first_arguments))
            else:
                cpp_struct_value.write("    TagClassInt {}_types[] = {{".format(member_name));
                for c in range(0, classes_len):
                    if c != 0:
                        cpp_struct_value.write(", ")
                    cpp_struct_value.write("TagClassInt::TAG_CLASS_{}".format(classes[c].upper()))
                cpp_struct_value.write("};\n");
                cpp_struct_value.write("    values.emplace_back({}, {}_types, {});\n".format(first_arguments, member_name, classes_len))
        elif type == "TagReflexive":
            minimum = 0 if not ("minimum" in struct) else struct["minimum"]
            maximum = 0xFFFFFFFF if not ("maximum" in struct) else struct["maximum"]

            vstruct = "std::vector<{}>".format(struct["struct"])
            cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::get_object_in_array_template<{}>, ParserStructValue::get_array_size_template<{}>, ParserStructValue::delete_objects_in_array_template<{}>, ParserStructValue::insert_object_in_array_template<{}>, ParserStructValue::duplicate_object_in_array_template<{}>, static_cast<std::size_t>({}), static_cast<std::size_t>({}));\n".format(first_arguments, vstruct, vstruct, vstruct, vstruct, vstruct, minimum, maximum))
        elif type == "TagDataOffset" or type == "TagString":
            cpp_struct_value.write("    values.emplace_back({});\n".format(first_arguments))
        elif type == "ScenarioScriptNodeValue" or type == "ScenarioStructureBSPArrayVertex":
            pass
        else:
            found = False
            for b in all_bitfields:
                if type == b["name"]:
                    found = True
                    type = "{}Enum".format(type)
                    cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::list_bitmask_template<HEK::{}, HEK::{}_to_string, {}>, ParserStructValue::list_bitmask_template<HEK::{}, HEK::{}_to_string_pretty, {}>, ParserStructValue::read_bitfield_template<HEK::{}, HEK::{}_from_string>, ParserStructValue::write_bitfield_template<HEK::{}, HEK::{}_from_string>);\n".format(first_arguments, type, type, len(b["fields_formatted"]), type, type, len(b["fields_formatted"]), type, type, type, type))
                    break
            if found:
                continue
            for e in all_enums:
                if type == e["name"]:
                    found = True
                    cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::list_enum_template<HEK::{}, HEK::{}_to_string, {}>, ParserStructValue::list_enum_template<HEK::{}, HEK::{}_to_string_pretty, {}>, ParserStructValue::read_enum_template<HEK::{}, HEK::{}_to_string>, ParserStructValue::write_enum_template<HEK::{}, HEK::{}_from_string>);\n".format(first_arguments, type, type, len(e["options_formatted"]), type, type, len(e["options_formatted"]), type, type, type, type))
                    break
            if found:
                continue

            bounds_b = "bounds" in struct and struct["bounds"]
            bounds = "true" if bounds_b else "false"
            count = 1 * (2 if bounds_b else 1)

            cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::ValueType::VALUE_TYPE_{}, {}, {});\n".format(first_arguments, type.upper(), count, bounds))

    cpp_struct_value.write("    return values;\n")
    cpp_struct_value.write("}\n")
    pass
