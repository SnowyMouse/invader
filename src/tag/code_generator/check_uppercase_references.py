# SPDX-License-Identifier: GPL-3.0-only

def make_check_uppercase_references(all_used_structs, struct_name, hpp, cpp_check_uppercase_references):
    hpp.write("        bool check_for_uppercase_references(bool fix_references) override;\n")
    cpp_check_uppercase_references.write("    bool {}::check_for_uppercase_references([[maybe_unused]] bool fix_references) {{\n".format(struct_name))
    cpp_check_uppercase_references.write("        bool return_value = false;\n")
    for struct in all_used_structs:
        if struct["type"] == "TagReflexive":
            cpp_check_uppercase_references.write("        for(auto &r : this->{}) {{\n".format(struct["member_name"]))
            cpp_check_uppercase_references.write("            return_value = r.check_for_uppercase_references(fix_references) || return_value;\n")
            cpp_check_uppercase_references.write("        }\n")
        if struct["type"] == "TagDependency":
            name = struct["member_name"];
            cpp_check_uppercase_references.write("        if(this->{}.path.size() > 0) {{\n".format(name))
            cpp_check_uppercase_references.write("            for(auto &i : this->{}.path) {{\n".format(name))
            cpp_check_uppercase_references.write("                auto lower_i = std::tolower(i);\n")
            cpp_check_uppercase_references.write("                if(lower_i != i) {\n")
            cpp_check_uppercase_references.write("                    if(!fix_references) {\n")
            cpp_check_uppercase_references.write("                        return true;\n")
            cpp_check_uppercase_references.write("                    }\n")
            cpp_check_uppercase_references.write("                    else {\n")
            cpp_check_uppercase_references.write("                        return_value = true;\n")
            cpp_check_uppercase_references.write("                        i = lower_i;\n")
            cpp_check_uppercase_references.write("                    }\n")
            cpp_check_uppercase_references.write("                }\n")
            cpp_check_uppercase_references.write("            }\n")
            cpp_check_uppercase_references.write("        }\n")
    cpp_check_uppercase_references.write("        return return_value;\n")
    cpp_check_uppercase_references.write("    }\n")
