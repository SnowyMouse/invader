# SPDX-License-Identifier: GPL-3.0-only

def make_check_broken_enums(all_enums, all_used_structs, struct_name, hpp, cpp_check_broken_enums):
    hpp.write("        bool check_for_broken_enums(bool reset_enums) override;\n")
    cpp_check_broken_enums.write("    bool {}::check_for_broken_enums([[maybe_unused]] bool reset_enums) {{\n".format(struct_name))
    cpp_check_broken_enums.write("        bool return_value = false;\n")
    for struct in all_used_structs:
        if struct["type"] == "TagReflexive":
            cpp_check_broken_enums.write("        for(auto &r : this->{}) {{\n".format(struct["member_name"]))
            cpp_check_broken_enums.write("            return_value = r.check_for_broken_enums(reset_enums) || return_value;\n")
            cpp_check_broken_enums.write("            if(!reset_enums && return_value) {\n")
            cpp_check_broken_enums.write("                return true;\n")
            cpp_check_broken_enums.write("            }\n")
            cpp_check_broken_enums.write("        }\n")
        for enum in all_enums:
            if struct["type"] == enum["name"]:
                cpp_check_broken_enums.write("        if(this->{} >= {}) {{\n".format(struct["member_name"], len(enum["options"])))
                cpp_check_broken_enums.write("            return_value = true;\n")
                cpp_check_broken_enums.write("            if(reset_enums) {\n")
                cpp_check_broken_enums.write("                this->{} = {{}};\n".format(struct["member_name"]))
                cpp_check_broken_enums.write("            }\n")
                cpp_check_broken_enums.write("            else {\n")
                cpp_check_broken_enums.write("                return true;\n")
                cpp_check_broken_enums.write("            }\n")
                cpp_check_broken_enums.write("        }\n")
                break
    cpp_check_broken_enums.write("        return return_value;\n")
    cpp_check_broken_enums.write("    }\n")
