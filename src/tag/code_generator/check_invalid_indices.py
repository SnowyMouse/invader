# SPDX-License-Identifier: GPL-3.0-only

def make_check_invalid_indices(all_used_structs, struct_name, hpp, cpp_check_invalid_indices, all_structs_arranged):
    hpp.write("        bool check_for_invalid_indices(bool null_indices) override;\n")
    cpp_check_invalid_indices.write("    bool {}::check_for_invalid_indices([[maybe_unused]] bool null_indices) {{\n".format(struct_name))
    cpp_check_invalid_indices.write("        std::deque<std::tuple<const ParserStruct *, std::size_t, const char *>> stack;\n")
    cpp_check_invalid_indices.write("        stack.emplace_front(this, 0, \"tag\");\n")
    cpp_check_invalid_indices.write("        return this->check_for_invalid_indices(null_indices, stack);\n")
    cpp_check_invalid_indices.write("    }\n")
                                                                                                                
    hpp.write("        bool check_for_invalid_indices(bool null_indices, std::deque<std::tuple<const ParserStruct *, std::size_t, const char *>> &stack) override;\n")
    cpp_check_invalid_indices.write("    bool {}::check_for_invalid_indices([[maybe_unused]] bool null_indices, std::deque<std::tuple<const ParserStruct *, std::size_t, const char *>> &stack) {{\n".format(struct_name))
    cpp_check_invalid_indices.write("        bool return_value = false;\n")
    for struct in all_used_structs:
        name = struct["member_name"]
        if struct["type"] == "TagReflexive":
            cpp_check_invalid_indices.write("        std::size_t {}_count = this->{}.size();\n".format(name, name))
            cpp_check_invalid_indices.write("        for(std::size_t i = 0; i < {}_count; i++) {{\n".format(name))
            cpp_check_invalid_indices.write("            auto *v = this->{}.data() + i;\n".format(name))
            cpp_check_invalid_indices.write("            stack.emplace_front(v, i, \"{}\");\n".format(name))
            cpp_check_invalid_indices.write("            if((return_value = v->check_for_invalid_indices(null_indices, stack) || return_value) && !null_indices) {\n")
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
            cpp_check_invalid_indices.write("            [[maybe_unused]] bool found = false;\n")
            cpp_check_invalid_indices.write("            for(auto &s : stack) {\n")
            cpp_check_invalid_indices.write("                auto *p = dynamic_cast<const {} *>(std::get<0>(s));\n".format(struct_to_check))
            cpp_check_invalid_indices.write("                if(p) {\n")
            cpp_check_invalid_indices.write("                    found = true;\n")
            cpp_check_invalid_indices.write("                    auto count = p->{}.size();\n".format(member_to_check))
            cpp_check_invalid_indices.write("                    if(this->{} >= count) {{\n".format(name))
            cpp_check_invalid_indices.write("                        if(null_indices) {\n")
            cpp_check_invalid_indices.write("                            return_value = true;\n")
            cpp_check_invalid_indices.write("                            oprintf_success_lesser_warn(\"    Nulled {}::{} ({}::{} #%zu >= %zu)\", static_cast<std::size_t>(this->{}), count);\n".format(struct_name, name, struct_to_check, member_to_check, name))
            cpp_check_invalid_indices.write("                            oprintf_success_lesser_warn(\"    Stack:\");\n")
            cpp_check_invalid_indices.write("                            auto stack_size = stack.size();\n")
            cpp_check_invalid_indices.write("                            for(std::size_t i = 0; i < stack_size - 1; i++) {\n")
            cpp_check_invalid_indices.write("                                auto *parent_struct_name = std::get<0>(stack[i + 1])->struct_name();\n")
            cpp_check_invalid_indices.write("                                auto *member_name = std::get<2>(stack[i]);\n")
            cpp_check_invalid_indices.write("                                auto index = std::get<1>(stack[i]);\n")
            cpp_check_invalid_indices.write("                                oprintf_success_lesser_warn(\"        %s::%s #%zu\", parent_struct_name, member_name, index);\n")
            cpp_check_invalid_indices.write("                            }\n")
            cpp_check_invalid_indices.write("                            this->{} = NULL_INDEX;\n".format(name))
            cpp_check_invalid_indices.write("                        }\n")
            cpp_check_invalid_indices.write("                        else {\n")
            cpp_check_invalid_indices.write("                            stack.erase(stack.begin());\n")
            cpp_check_invalid_indices.write("                            return true;\n")
            cpp_check_invalid_indices.write("                        }\n")
            cpp_check_invalid_indices.write("                    }\n")
            cpp_check_invalid_indices.write("                    break;\n")
            cpp_check_invalid_indices.write("                }\n")
            cpp_check_invalid_indices.write("            }\n")
            cpp_check_invalid_indices.write("            #ifndef NDEBUG\n")
            cpp_check_invalid_indices.write("            if(!found) {\n")
            cpp_check_invalid_indices.write("                eprintf_warn(\"DEBUG: {} was not found in the stack when checking {}::{}'s index.\");\n".format(struct_to_check, struct_name, name))
            cpp_check_invalid_indices.write("            }\n")
            cpp_check_invalid_indices.write("            #endif\n")
            cpp_check_invalid_indices.write("        }\n")
        
    cpp_check_invalid_indices.write("        stack.erase(stack.begin());\n")
    cpp_check_invalid_indices.write("        return return_value;\n")
    cpp_check_invalid_indices.write("    }\n")
