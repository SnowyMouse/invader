# SPDX-License-Identifier: GPL-3.0-only

def make_normalize(all_used_structs, struct_name, hpp, cpp_normalize):
    hpp.write("        bool check_for_nonnormal_vectors(bool normalize) override;\n")
    cpp_normalize.write("    bool {}::check_for_nonnormal_vectors([[maybe_unused]] bool normalize) {{\n".format(struct_name))
    cpp_normalize.write("        bool return_value = false;\n")
    for struct in all_used_structs:
        name = struct["member_name"]
        if (struct["type"] != "TagReflexive") and ((not "normalize" in struct) or (not struct["normalize"])):
            continue
        
        if struct["type"] == "TagReflexive":
            cpp_normalize.write("        for(auto &r : this->{}) {{\n".format(name))
            cpp_normalize.write("            return_value = r.check_for_nonnormal_vectors(normalize) || return_value;\n")
            cpp_normalize.write("        }\n")
        elif (struct["type"] == "Vector2D" or struct["type"] == "Vector3D" or struct["type"] == "Quaternion"):
            cpp_normalize.write("        if(!this->{}.is_normalized()) {{\n".format(name))
            cpp_normalize.write("            if(!normalize) {\n")
            cpp_normalize.write("                return true;\n")
            cpp_normalize.write("            }\n")
            cpp_normalize.write("            else {\n")
            cpp_normalize.write("                this->{} = this->{}.normalize();\n".format(name, name))
            cpp_normalize.write("                return_value = true;\n")
            cpp_normalize.write("            }\n")
            cpp_normalize.write("        }\n")
        elif (struct["type"] == "Plane2D" or struct["type"] == "Plane3D"):
            cpp_normalize.write("        if(!this->{}.vector.is_normalized()) {{\n".format(name))
            cpp_normalize.write("            if(!normalize) {\n")
            cpp_normalize.write("                return true;\n")
            cpp_normalize.write("            }\n")
            cpp_normalize.write("            else {\n")
            cpp_normalize.write("                this->{}.vector = this->{}.vector.normalize();\n".format(name, name))
            cpp_normalize.write("                return_value = true;\n")
            cpp_normalize.write("            }\n")
            cpp_normalize.write("        }\n")
        
    cpp_normalize.write("        return return_value;\n")
    cpp_normalize.write("    }\n")
