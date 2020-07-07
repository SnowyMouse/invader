# SPDX-License-Identifier: GPL-3.0-only

import sys

def make_cache_format_data(struct_name, s, pre_compile, post_compile, all_used_structs, hpp, cpp_cache_format_data, all_enums, all_structs_arranged):
    # compile()
    hpp.write("        void compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp = std::nullopt, std::size_t offset = 0, std::deque<const ParserStruct *> *stack = nullptr) override;\n")
    cpp_cache_format_data.write("    void {}::compile(BuildWorkload &workload, [[maybe_unused]] std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp, std::size_t offset, std::deque<const ParserStruct *> *stack) {{\n".format(struct_name))
    
    # Make the stack if we need it
    cpp_cache_format_data.write("        std::optional<std::deque<const ParserStruct *>> new_stack;\n")
    cpp_cache_format_data.write("        if(!stack) {\n")
    cpp_cache_format_data.write("            new_stack = std::deque<const ParserStruct *>();\n")
    cpp_cache_format_data.write("            stack = &*new_stack;\n")
    cpp_cache_format_data.write("        }\n")
    
    ## Add our struct to the stack
    cpp_cache_format_data.write("        stack->push_front(this);\n")
    
    # Zero out the base struct
    cpp_cache_format_data.write("        auto *start = workload.structs[struct_index].data.data();\n")
    cpp_cache_format_data.write("        workload.structs[struct_index].bsp = bsp;\n")
    cpp_cache_format_data.write("        workload.structs[struct_index].unsafe_to_dedupe = {};\n".format("true" if ("unsafe_to_dedupe" in s and s["unsafe_to_dedupe"]) else "false"))
    if pre_compile:
        cpp_cache_format_data.write("        if(!this->cache_formatted) {\n")
        cpp_cache_format_data.write("            this->pre_compile(workload, tag_index, struct_index, offset);\n")
        cpp_cache_format_data.write("        }\n")
        cpp_cache_format_data.write("        this->cache_formatted = true;\n")
    cpp_cache_format_data.write("        auto &r = *reinterpret_cast<struct_little *>(start + offset);\n")
    cpp_cache_format_data.write("        std::fill(reinterpret_cast<std::byte *>(&r), reinterpret_cast<std::byte *>(&r), std::byte());\n")
    
    # Go through each field
    for struct in all_used_structs:
        if ("non_cached" in struct and struct["non_cached"]) or ("compile_ignore" in struct and struct["compile_ignore"]):
            continue
        name = struct["member_name"]
        minimum = struct["minimum"] if "minimum" in struct else None
        maximum = struct["maximum"] if "maximum" in struct else None
        if struct["type"] == "TagDependency":
            cpp_cache_format_data.write("        this->{}.tag_id = HEK::TagID::null_tag_id();\n".format(name))
            cpp_cache_format_data.write("        r.{}.tag_class_int = this->{}.tag_class_int;\n".format(name, name))
            cpp_cache_format_data.write("        if(this->{}.path.size() > 0) {{\n".format(name))

            # Make sure the class is correct for the reference
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

                cpp_cache_format_data.write("            if({}) {{\n".format(test_line))
                cpp_cache_format_data.write("                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} must be{}, found %s, instead\", tag_class_to_extension(this->{}.tag_class_int));\n".format(struct_name, name, error_line, name))
                cpp_cache_format_data.write("                throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("            }\n")

            cpp_cache_format_data.write("            std::size_t index = workload.compile_tag_recursively(this->{}.path.c_str(), this->{}.tag_class_int);\n".format(name, name))
            cpp_cache_format_data.write("            this->{}.tag_id.index = static_cast<std::uint16_t>(index);\n".format(name))
            cpp_cache_format_data.write("            r.{}.tag_id = this->{}.tag_id;\n".format(name, name))
            cpp_cache_format_data.write("            auto &d = workload.structs[struct_index].dependencies.emplace_back();\n")
            cpp_cache_format_data.write("            d.offset = reinterpret_cast<std::byte *>(&r.{}) - start;\n".format(name))
            cpp_cache_format_data.write("            d.tag_index = index;\n".format(name))
            cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        else {\n")
            if "non_null" in struct and struct["non_null"]:
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must not be null\", tag_index);\n".format(struct_name, name))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
            else:
                cpp_cache_format_data.write("            r.{}.tag_id = HEK::TagID::null_tag_id();\n".format(name))
            cpp_cache_format_data.write("        }\n")
        elif struct["type"] == "TagReflexive":
            cpp_cache_format_data.write("        std::size_t t_{}_count = this->{}.size();\n".format(name, name))
            
            # Make sure we're within bounds in the reflexive
            if minimum != None:
                cpp_cache_format_data.write("        if(t_{}_count < {}) {{\n".format(name, minimum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must have at least {} block{}\", tag_index);\n".format(struct_name, name, minimum, "" if minimum == 1 else "s"))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            if maximum != None:
                cpp_cache_format_data.write("        if(t_{}_count > {}) {{\n".format(name, maximum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must have no more than {} block{}\", tag_index);\n".format(struct_name, name, maximum, "" if maximum == 1 else "s"))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
                
            # Now actually work
            cpp_cache_format_data.write("        if(t_{}_count > 0) {{\n".format(name))
            cpp_cache_format_data.write("            r.{}.count = static_cast<std::uint32_t>(t_{}_count);\n".format(name, name))
            cpp_cache_format_data.write("            auto &n = workload.structs.emplace_back();\n")
            cpp_cache_format_data.write("            static constexpr std::size_t STRUCT_SIZE = sizeof({}::struct_little);\n".format(struct["struct"]))
            cpp_cache_format_data.write("            n.data.resize(t_{}_count * STRUCT_SIZE);\n".format(name))
            cpp_cache_format_data.write("            auto &p = workload.structs[struct_index].pointers.emplace_back();\n")
            cpp_cache_format_data.write("            p.struct_index = &n - workload.structs.data();\n")
            cpp_cache_format_data.write("            p.offset = reinterpret_cast<std::byte *>(&r.{}.pointer) - start;\n".format(name))
            cpp_cache_format_data.write("            for(std::size_t i = 0; i < t_{}_count; i++) {{\n".format(name))
            cpp_cache_format_data.write("                try {\n")
            cpp_cache_format_data.write("                    this->{}[i].compile(workload, tag_index, p.struct_index, bsp, i * STRUCT_SIZE, stack);\n".format(name))
            cpp_cache_format_data.write("                }\n")
            cpp_cache_format_data.write("                catch(std::exception &) {\n")
            cpp_cache_format_data.write("                    eprintf(\"Failed to compile {}::{} #%zu\\n\", i);\n".format(struct_name, name))
            cpp_cache_format_data.write("                    throw;\n")
            cpp_cache_format_data.write("                }\n")
            cpp_cache_format_data.write("            }\n")
            cpp_cache_format_data.write("        }\n")
        elif struct["type"] == "TagDataOffset":
            cpp_cache_format_data.write("        std::size_t t_{}_size = this->{}.size();\n".format(name, name))
            cpp_cache_format_data.write("        if(t_{}_size > 0) {{\n".format(name))
            cpp_cache_format_data.write("            auto &n = workload.structs.emplace_back();\n")
            cpp_cache_format_data.write("            n.bsp = bsp;\n")
            cpp_cache_format_data.write("            n.data.insert(n.data.begin(), this->{}.begin(), this->{}.end());\n".format(name, name))
            cpp_cache_format_data.write("            auto &p = workload.structs[struct_index].pointers.emplace_back();\n")
            cpp_cache_format_data.write("            p.struct_index = &n - workload.structs.data();\n")
            cpp_cache_format_data.write("            p.offset = reinterpret_cast<std::byte *>(&r.{}.pointer) - start;\n".format(name))
            cpp_cache_format_data.write("            r.{}.size = t_{}_size;\n".format(name, name))
            cpp_cache_format_data.write("        }\n")
        elif "bounds" in struct and struct["bounds"]:
            # Make sure the value is within bounds
            if minimum != None:
                cpp_cache_format_data.write("        if(!workload.disable_recursion && (this->{}.from < {} || this->{}.to < {})) {{\n".format(name, minimum, name, minimum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must be at least {}\", tag_index);\n".format(struct_name, name, minimum))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            if maximum != None:
                cpp_cache_format_data.write("        if(!workload.disable_recursion && (this->{}.from > {} || this->{}.to > {})) {{\n".format(name, maximum, name, maximum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must no more than {}\", tag_index);\n".format(struct_name, name, minimum))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        r.{}.from = this->{}.from;\n".format(name, name))
            cpp_cache_format_data.write("        r.{}.to = this->{}.to;\n".format(name, name))
        elif "count" in struct and struct["count"] > 1:
            cpp_cache_format_data.write("        std::copy(this->{}, this->{} + {}, r.{});\n".format(name, name, struct["count"], name))
        elif struct["type"] == "Index":
            reflexive_to_check = struct["reflexive"] if "reflexive" in struct else None
            struct_to_check = struct["struct"] if "struct" in struct else None
            if reflexive_to_check != None and struct_to_check != None:
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
                
                cpp_cache_format_data.write("        if(!workload.disable_recursion && this->{} != NULL_INDEX) {{\n".format(name))
                cpp_cache_format_data.write("            [[maybe_unused]] bool found = false;\n")
                
                def do_it_for_sam(member_to_check):
                    cpp_cache_format_data.write("            for(auto *p : *stack) {\n")
                    cpp_cache_format_data.write("                auto *s = dynamic_cast<const {} *>(p);\n".format(struct_to_check))
                    cpp_cache_format_data.write("                if(s) {\n")
                    cpp_cache_format_data.write("                    auto count = s->{}.size();\n".format(member_to_check))
                    cpp_cache_format_data.write("                    if(this->{} >= count) {{\n".format(name))
                    cpp_cache_format_data.write("                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} references an invalid index of {}::{} (%zu >= %zu)\", static_cast<std::size_t>(this->{}), count);\n".format(struct_name, name, struct_to_check, member_to_check, name))
                    cpp_cache_format_data.write("                        throw InvalidTagDataException();\n")
                    cpp_cache_format_data.write("                    }\n")
                    cpp_cache_format_data.write("                    found = true;\n")
                    cpp_cache_format_data.write("                    break;\n")
                    cpp_cache_format_data.write("                }\n")
                    cpp_cache_format_data.write("            }\n")
                    
                do_it_for_sam(member_to_check)
                
                # Also check GBXModel too if necessary
                if member_to_check == "Model":
                    do_it_for_sam("GBXModel")
                    
                cpp_cache_format_data.write("            #ifndef NDEBUG\n")
                cpp_cache_format_data.write("            if(!found) {\n")
                cpp_cache_format_data.write("                eprintf_warn(\"DEBUG: {} was not found in the stack when checking {}::{}'s index.\");\n".format(struct_to_check, struct_name, name))
                cpp_cache_format_data.write("            }\n")
                cpp_cache_format_data.write("            #endif\n")
                cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        r.{} = this->{};\n".format(name, name))
        else:
            for e in all_enums:
                if e["name"] == struct["type"]:
                    shifted_by_one = "+ 1" if ("shifted_by_one" in struct and struct["shifted_by_one"]) else ""
                    cpp_cache_format_data.write("        if(!workload.disable_recursion && static_cast<std::uint16_t>(this->{}{}) >= {}) {{\n".format(name, shifted_by_one, len(e["options_formatted"])))
                    cpp_cache_format_data.write("            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} is out of range (%zu >= {})\", static_cast<std::size_t>(static_cast<std::uint16_t>(this->{}{})));\n".format(struct_name, name, len(e["options_formatted"]), name, shifted_by_one))
                    cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                    cpp_cache_format_data.write("        }\n")
                    break
            # Make sure the value is within bounds
            if minimum != None:
                cpp_cache_format_data.write("        if(!workload.disable_recursion && this->{} < {}) {{\n".format(name, minimum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must be at least {}\", tag_index);\n".format(struct_name, name, minimum))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            if maximum != None:
                cpp_cache_format_data.write("        if(!workload.disable_recursion && this->{} > {}) {{\n".format(name, maximum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must no more than {}\", tag_index);\n".format(struct_name, name, maximum))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        r.{} = this->{};\n".format(name, name))
    if post_compile:
        cpp_cache_format_data.write("        this->post_compile(workload, tag_index, struct_index, offset);\n".format(name, name))
    
    ## Remove our struct from the top of the stack
    cpp_cache_format_data.write("        stack->erase(stack->begin());\n")
    cpp_cache_format_data.write("    }\n")
