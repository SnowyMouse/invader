# SPDX-License-Identifier: GPL-3.0-only

def make_check_invalid_indices(all_used_structs, struct_name, hpp, cpp_check_invalid_indices, all_structs_arranged):
    hpp.write("        bool check_for_invalid_indices(bool null_indices) override;\n")
    cpp_check_invalid_indices.write("    bool {}::check_for_invalid_indices([[maybe_unused]] bool null_indices) {{\n".format(struct_name))
    cpp_check_invalid_indices.write("        std::deque<const ParserStruct *> stack;\n")
    cpp_check_invalid_indices.write("        return this->check_for_invalid_indices(null_indices, stack);\n")
    cpp_check_invalid_indices.write("    }\n")
                                                                                                                
    hpp.write("        bool check_for_invalid_indices(bool null_indices, std::deque<const ParserStruct *> &stack) override;\n")
    cpp_check_invalid_indices.write("    bool {}::check_for_invalid_indices([[maybe_unused]] bool null_indices, std::deque<const ParserStruct *> &stack) {{\n".format(struct_name))
    cpp_check_invalid_indices.write("        bool return_value = false;\n")
    cpp_check_invalid_indices.write("        stack.push_front(this);\n")
    for struct in all_used_structs:
        name = struct["member_name"]
        if struct["type"] == "TagReflexive":
            cpp_check_invalid_indices.write("        for(auto &r : this->{}) {{\n".format(name))
            cpp_check_invalid_indices.write("            if((return_value = r.check_for_invalid_indices(null_indices, stack) || return_value) && !null_indices) {\n")
            cpp_check_invalid_indices.write("                stack.erase(stack.begin());\n")
            cpp_check_invalid_indices.write("                return true;\n")
            cpp_check_invalid_indices.write("            }\n")
            cpp_check_invalid_indices.write("        }\n")
        elif struct["type"] == "Index":
            reflexive_to_check = struct["reflexive"] if "reflexive" in struct else None
            struct_to_check = struct["struct"] if "struct" in struct else None
            if reflexive_to_check is None and struct_to_check is None:
                continue
            
            member_to_check = None
            for i in all_structs_arranged:
                if i["name"] == struct_to_check:
                    for p in i["fields"]:
                        if "name" in p and p["name"] == reflexive_to_check:
                            member_to_check = p["member_name"]
                            break
                    break
            
            if member_to_check is None:
                print("Cannot resolve {} in {}".format(reflexive_to_check, struct_to_check), file=sys.stderr)
                sys.exit(1)
        
            cpp_check_invalid_indices.write("        if(this->{} != NULL_INDEX) {{\n".format(name))
            cpp_check_invalid_indices.write("            for(auto *s : stack) {\n")
            cpp_check_invalid_indices.write("                auto *p = dynamic_cast<const {} *>(s);\n".format(struct_to_check))
            cpp_check_invalid_indices.write("                if(p) {\n")
            cpp_check_invalid_indices.write("                    if(this->{} >= p->{}.size()) {{\n".format(name, member_to_check))
            cpp_check_invalid_indices.write("                        if(null_indices) {\n")
            cpp_check_invalid_indices.write("                            return_value = true;\n")
            cpp_check_invalid_indices.write("                            this->{} = NULL_INDEX;\n".format(name))
            cpp_check_invalid_indices.write("                        }\n")
            cpp_check_invalid_indices.write("                        else {\n")
            cpp_check_invalid_indices.write("                            stack.erase(stack.begin());\n")
            cpp_check_invalid_indices.write("                            return true;\n")
            cpp_check_invalid_indices.write("                        }\n")
            cpp_check_invalid_indices.write("                    }\n")
            cpp_check_invalid_indices.write("                }\n")
            cpp_check_invalid_indices.write("            }\n")
            cpp_check_invalid_indices.write("        }\n")
        
    cpp_check_invalid_indices.write("        stack.erase(stack.begin());\n")
    cpp_check_invalid_indices.write("        return return_value;\n")
    cpp_check_invalid_indices.write("    }\n")
