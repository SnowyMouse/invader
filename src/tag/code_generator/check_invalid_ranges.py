# SPDX-License-Identifier: GPL-3.0-only

def make_check_invalid_ranges(all_used_structs, struct_name, hpp, cpp_check_invalid_ranges):
    hpp.write("        bool check_for_invalid_ranges(bool clamp) override;\n")
    cpp_check_invalid_ranges.write("    bool {}::check_for_invalid_ranges([[maybe_unused]] bool clamp) {{\n".format(struct_name))
    cpp_check_invalid_ranges.write("        bool return_value = false;\n")
    for struct in all_used_structs:
        name = struct["member_name"]
        if struct["type"] == "TagReflexive":
            cpp_check_invalid_ranges.write("        for(auto &r : this->{}) {{\n".format(name))
            cpp_check_invalid_ranges.write("            return_value = r.check_for_invalid_ranges(clamp) || return_value;\n")
            cpp_check_invalid_ranges.write("        }\n")
        elif struct["type"] == "TagDataOffset":
            pass
        else:
            # Get the min/max values
            minimum = struct["minimum"] if "minimum" in struct else None
            maximum = struct["maximum"] if "maximum" in struct else None
            if minimum is None and maximum is None:
                continue

            def write_range_check(what):
                if minimum != None:
                    cpp_check_invalid_ranges.write("        if({} < {}) {{\n".format(what, minimum))
                    cpp_check_invalid_ranges.write("            if(!clamp) {\n")
                    cpp_check_invalid_ranges.write("                return true;\n")
                    cpp_check_invalid_ranges.write("            }\n")
                    cpp_check_invalid_ranges.write("            {} = {};\n".format(what, minimum))
                    cpp_check_invalid_ranges.write("            return_value = true;\n")
                    cpp_check_invalid_ranges.write("        }\n")
                if maximum != None:
                    cpp_check_invalid_ranges.write("        if({} > {}) {{\n".format(what, maximum))
                    cpp_check_invalid_ranges.write("            if(!clamp) {\n")
                    cpp_check_invalid_ranges.write("                return true;\n")
                    cpp_check_invalid_ranges.write("            }\n")
                    cpp_check_invalid_ranges.write("            {} = {};\n".format(what, maximum))
                    cpp_check_invalid_ranges.write("            return_value = true;\n")
                    cpp_check_invalid_ranges.write("        }\n")

            if ("bounds" in struct) and struct["bounds"]:
                write_range_check("this->{}.from".format(name))
                write_range_check("this->{}.to".format(name))
            else:
                write_range_check("this->{}".format(name))

    cpp_check_invalid_ranges.write("        return return_value;\n")
    cpp_check_invalid_ranges.write("    }\n")
