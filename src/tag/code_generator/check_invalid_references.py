# SPDX-License-Identifier: GPL-3.0-only

def make_check_invalid_references(all_used_structs, struct_name, hpp, cpp_check_invalid_references):
    hpp.write("        bool check_for_invalid_references(bool null_references) override;\n")
    cpp_check_invalid_references.write("    bool {}::check_for_invalid_references([[maybe_unused]] bool null_references) {{\n".format(struct_name))
    cpp_check_invalid_references.write("        bool return_value = false;\n")
    for struct in all_used_structs:
        if struct["type"] == "TagReflexive":
            cpp_check_invalid_references.write("        for(auto &r : this->{}) {{\n".format(struct["member_name"]))
            cpp_check_invalid_references.write("            return_value = r.check_for_invalid_references(null_references) || return_value;\n")
            cpp_check_invalid_references.write("        }\n")
        if struct["type"] == "TagDependency":
            name = struct["member_name"];
            if struct["classes"][0] != "*":
                test_line = ""
                error_line = ""
                classes = struct["classes"]
                classes_len = len(classes)
                for c in range(0, classes_len):
                    if c != 0:
                        test_line = " && " + test_line
                    test_line = "this->{}.tag_class_int != TagClassInt::TAG_CLASS_{}".format(name, classes[c].upper()) + test_line
                if classes_len == 1:
                    error_line = " {}".format(classes[0])
                elif classes_len == 2:
                    error_line = " {} or {}".format(classes[0], classes[1])
                else:
                    for c in range(0, classes_len):
                        if c != 0:
                            error_line = error_line + ","
                        if c + 1 == classes_len:
                            error_line = error_line + " or"
                        error_line = error_line + " {}".format(classes[c])

                cpp_check_invalid_references.write("        if({}) {{\n".format(test_line))
                cpp_check_invalid_references.write("            if(!null_references) {\n")
                cpp_check_invalid_references.write("                return true;\n")
                cpp_check_invalid_references.write("            }\n")
                cpp_check_invalid_references.write("            this->{}.tag_class_int = TagClassInt::TAG_CLASS_{};\n".format(name, classes[0].upper()))
                cpp_check_invalid_references.write("            this->{}.path = std::string();\n".format(name))
                cpp_check_invalid_references.write("            return_value = true;\n".format(name))
                cpp_check_invalid_references.write("        }\n")
    cpp_check_invalid_references.write("        return return_value;\n")
    cpp_check_invalid_references.write("    }\n")
