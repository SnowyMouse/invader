# SPDX-License-Identifier: GPL-3.0-only

def make_parser_struct(cpp_struct_value, all_used_structs, hpp, struct_name, extract_hidden):
    hpp.write("        std::vector<ParserStructValue> get_values() override;\n".format(struct_name))
    cpp_struct_value.write("std::vector<ParserStructValue> {}::get_values() {{\n".format(struct_name))
    cpp_struct_value.write("    std::vector<ParserStructValue> values;\n")
    for struct in all_used_structs:
        if "cache_only" in struct and struct["cache_only"] and not extract_hidden:
            continue

        name = "\"{}\"".format(struct["name"])
        member_name = struct["member_name"]
        member_name_q = "\"{}\"".format(member_name)
        comment = "nullptr" if "comment" not in struct else "\"{}\"".format(struct["comment"])

        first_arguments = "{},{},{},&this->{}".format(name, member_name_q, comment, struct["member_name"])

        if struct["type"] == "TagDependency":
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
        elif struct["type"] == "TagReflexive":
            vstruct = "std::vector<{}>".format(struct["struct"])
            cpp_struct_value.write("    values.emplace_back({}, ParserStructValue::get_object_in_array_template<{}>, ParserStructValue::get_array_size_template<{}>, ParserStructValue::delete_objects_in_array_template<{}>, ParserStructValue::insert_object_in_array_template<{}>, ParserStructValue::duplicate_object_in_array_template<{}>);\n".format(first_arguments, vstruct, vstruct, vstruct, vstruct, vstruct))

    cpp_struct_value.write("    return values;\n")
    cpp_struct_value.write("}\n")
    pass
