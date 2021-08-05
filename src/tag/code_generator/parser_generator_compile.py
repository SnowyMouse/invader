# SPDX-License-Identifier: GPL-3.0-only

def make_compile(definition, all_types, append_line):
    for s in definition["definitions"]:
        if s["type"] == "struct":
            struct_name = s["name"]
            
            append_line("void {}::compile(BuildWorkload &workload, [[maybe_unused]] std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp, std::size_t offset, std::deque<const ParserStruct *> *stack) {{".format(struct_name))

            # If we inherit anything, compile that too
            if "inherits" in s:
                append_line("dynamic_cast<{} *>(this)->compile(workload, tag_index, struct_index, bsp, offset, stack);".format(s["inherits"]), 1)

            # Make the stack if we need it
            append_line("std::optional<std::deque<const ParserStruct *>> new_stack;", 1)
            append_line("if(!stack) {", 1)
            append_line("new_stack = std::deque<const ParserStruct *>();", 2)
            append_line("stack = &*new_stack;", 2)
            append_line("}", 1)

            ## Add our struct to the stack
            append_line("stack->push_front(this);", 1)

            # Check if we're on the native engine. if not, check against stock limits (but don't strictly error)
            append_line("[[maybe_unused]] auto check_stock_limits = workload.get_build_parameters()->details.build_cache_file_engine != CacheFileEngine::CACHE_FILE_NATIVE;", 1)

            # Zero out the base struct
            append_line("auto *start = workload.structs[struct_index].data.data();", 1)
            append_line("workload.structs[struct_index].bsp = bsp;", 1)
            append_line("workload.structs[struct_index].unsafe_to_dedupe = {};".format("true" if ("unsafe_to_dedupe" in s and s["unsafe_to_dedupe"]) else "false"), 1)
            if "pre_compile" in s and s["pre_compile"]:
                append_line("if(!this->cache_formatted) {", 1)
                append_line("this->pre_compile(workload, tag_index, struct_index, offset);", 2)
                append_line("}", 1)
                append_line("this->cache_formatted = true;", 1)
            append_line("auto &r = *reinterpret_cast<C<LittleEndian> *>(start + offset);", 1)
            append_line("std::fill(reinterpret_cast<std::byte *>(&r), reinterpret_cast<std::byte *>(&r), std::byte());", 1)

            # Go through each field
            for struct in s["fields"]:
                if struct["type"] == "pad":
                    continue
                if ("non_cached" in struct and struct["non_cached"]) or ("compile_ignore" in struct and struct["compile_ignore"]):
                    continue
                name = struct["member_name"]
                minimum = struct["minimum"] if "minimum" in struct else None
                maximum = struct["maximum"] if "maximum" in struct else None
                if struct["type"] == "TagDependency":
                    append_line("this->{}.tag_id = TagID::null_tag_id();".format(name), 1)
                    append_line("r.{}.tag_fourcc = this->{}.tag_fourcc;".format(name, name), 1)
                    append_line("if(this->{}.path.size() > 0) {{".format(name), 1)

                    # Make sure the class is correct for the reference
                    if struct["classes"][0] != "*":
                        test_line = ""
                        error_line = ""
                        classes = struct["classes"]
                        classes_len = len(classes)
                        for c in range(0, classes_len):
                            if c != 0:
                                test_line = " && " + test_line
                            test_line = "this->{}.tag_fourcc != TagFourCC::TAG_FOURCC_{}".format(name, classes[c].upper()) + test_line
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

                        append_line("if({}) {{".format(test_line), 2)
                        append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} must be{}, found %s, instead\", tag_fourcc_to_extension(this->{}.tag_fourcc));".format(struct_name, name, error_line, name), 3)
                        append_line("throw InvalidTagDataException();", 3)
                        append_line("}", 2)

                    append_line("std::size_t index = workload.compile_tag_recursively(this->{}.path.c_str(), this->{}.tag_fourcc);".format(name, name), 2)
                    append_line("this->{}.tag_id.index = static_cast<std::uint16_t>(index);".format(name), 2)
                    append_line("r.{}.tag_id = this->{}.tag_id;".format(name, name), 2)
                    append_line("auto &d = workload.structs[struct_index].dependencies.emplace_back();", 2)
                    append_line("d.offset = reinterpret_cast<std::byte *>(&r.{}) - start;".format(name), 2)
                    append_line("d.tag_index = index;".format(name), 2)
                    append_line("}", 1)
                    append_line("else {", 1)
                    if "non_null" in struct and struct["non_null"]:
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must not be null\", tag_index);".format(struct_name, name), 2)
                        append_line("throw InvalidTagDataException();", 2)
                    else:
                        append_line("r.{}.tag_id = TagID::null_tag_id();".format(name), 2)
                    append_line("}", 1)
                elif struct["type"] == "TagReflexive":
                    append_line("std::size_t t_{}_count = this->{}.size();".format(name, name), 1)
                    
                    # Make sure we're within bounds in the reflexive
                    if minimum != None:
                        append_line("if(t_{}_count < {}) {{".format(name, minimum), 1)
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must have at least {} block{}\", tag_index);".format(struct_name, name, minimum, "" if minimum == 1 else "s"), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                    if maximum != None:
                        append_line("if(t_{}_count > {}) {{".format(name, maximum), 1)
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must have no more than {} block{}\", tag_index);".format(struct_name, name, maximum, "" if maximum == 1 else "s"), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                        
                    # If there's a limited defined by the HEK, warn
                    if "hek_maximum" in struct:
                        append_line("if(check_stock_limits && t_{}_count > {}) {{".format(name, struct["hek_maximum"]), 1)
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, \"{}::{} exceeds the stock limit of {} block{} and may not work as intended on the target engine\", tag_index);".format(struct_name, name, struct["hek_maximum"], "" if struct["hek_maximum"] == 1 else "s"), 2)
                        append_line("}", 1)
                        
                    # Now actually work
                    append_line("if(t_{}_count > 0) {{".format(name), 1)
                    append_line("r.{}.count = static_cast<std::uint32_t>(t_{}_count);".format(name, name), 2)
                    append_line("auto &n = workload.structs.emplace_back();", 2)
                    append_line("static constexpr std::size_t STRUCT_SIZE = sizeof({}::C<LittleEndian>);".format(struct["struct"]), 2)
                    append_line("n.data.resize(t_{}_count * STRUCT_SIZE);".format(name), 2)
                    append_line("auto &p = workload.structs[struct_index].pointers.emplace_back();", 2)
                    append_line("p.struct_index = &n - workload.structs.data();", 2)
                    append_line("p.offset = reinterpret_cast<std::byte *>(&r.{}.pointer) - start;".format(name), 2)
                    append_line("for(std::size_t i = 0; i < t_{}_count; i++) {{".format(name), 2)
                    append_line("try {", 3)
                    append_line("this->{}[i].compile(workload, tag_index, p.struct_index, bsp, i * STRUCT_SIZE, stack);".format(name), 4)
                    append_line("}", 3)
                    append_line("catch(std::exception &) {", 3)
                    append_line("eprintf(\"Failed to compile {}::{} #%zu\\n\", i);".format(struct_name, name), 4)
                    append_line("throw;", 4)
                    append_line("}", 3)
                    append_line("}", 2)
                    append_line("}", 1)
                elif struct["type"] == "TagDataOffset":
                    append_line("std::size_t t_{}_size = this->{}.size();".format(name, name), 1)
                    append_line("if(t_{}_size > 0) {{".format(name), 1)
                    append_line("auto &n = workload.structs.emplace_back();", 2)
                    append_line("n.bsp = bsp;", 2)
                    append_line("n.data.insert(n.data.begin(), this->{}.begin(), this->{}.end());".format(name, name), 2)
                    append_line("auto &p = workload.structs[struct_index].pointers.emplace_back();", 2)
                    append_line("p.struct_index = &n - workload.structs.data();", 2)
                    append_line("p.offset = reinterpret_cast<std::byte *>(&r.{}.pointer) - start;".format(name), 2)
                    append_line("r.{}.size = t_{}_size;".format(name, name), 2)
                    append_line("}", 1)
                elif "bounds" in struct and struct["bounds"]:
                    # Make sure the value is within bounds
                    if minimum != None:
                        append_line("if(!workload.disable_error_checking && (this->{}.from < {} || this->{}.to < {})) {{".format(name, minimum, name, minimum), 1)
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must be at least {}\", tag_index);".format(struct_name, name, minimum), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                    if maximum != None:
                        append_line("if(!workload.disable_error_checking && (this->{}.from > {} || this->{}.to > {})) {{".format(name, maximum, name, maximum), 1)
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must no more than {}\", tag_index);".format(struct_name, name, minimum), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                    append_line("r.{}.from = this->{}.from;".format(name, name), 1)
                    append_line("r.{}.to = this->{}.to;".format(name, name), 1)
                elif "count" in struct and struct["count"] > 1:
                    append_line("std::copy(this->{}, this->{} + {}, r.{});".format(name, name, struct["count"], name), 1)
                elif struct["type"] == "Index":
                    reflexive_to_check = struct["reflexive"] if "reflexive" in struct else None
                    struct_to_check = struct["struct"] if "struct" in struct else None
                    member_to_check = None
                    
                    if reflexive_to_check != None and struct_to_check != None:
                        # Find the member and resolve it
                        for f in all_types[struct_to_check]["fields"]:
                            if f["type"] != "pad" and f["name"] == reflexive_to_check:
                                member_to_check = f["member_name"]
                                break
                        
                        append_line("if(!workload.disable_error_checking && this->{} != NULL_INDEX) {{".format(name), 1)
                        append_line("[[maybe_unused]] bool found = false;", 2)
                        
                        def do_it_for_sam(struct_to_check):
                            append_line("for(auto *p : *stack) {", 2)
                            append_line("auto *s = dynamic_cast<const {} *>(p);".format(struct_to_check), 3)
                            append_line("if(s) {", 3)
                            append_line("auto count = s->{}.size();".format(member_to_check), 4)
                            append_line("if(this->{} >= count) {{".format(name), 4)
                            append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} references an invalid index of {}::{} (%zu >= %zu)\", static_cast<std::size_t>(this->{}), count);".format(struct_name, name, struct_to_check, member_to_check, name), 5)
                            append_line("throw InvalidTagDataException();", 5)
                            append_line("}", 4)
                            append_line("found = true;", 4)
                            append_line("break;", 4)
                            append_line("}", 3)
                            append_line("}", 2)
                            
                        do_it_for_sam(struct_to_check)
                        
                        # Also check GBXModel too if necessary
                        if struct_to_check == "Model":
                            do_it_for_sam("GBXModel")
                            
                        append_line("#ifndef NDEBUG", 2)
                        append_line("if(!found) {", 2)
                        append_line("eprintf_warn(\"DEBUG: {} was not found in the stack when checking {}::{}'s index.\");".format(struct_to_check, struct_name, name), 3)
                        append_line("}", 2)
                        append_line("#endif", 2)
                        append_line("}", 1)
                    append_line("r.{} = this->{};".format(name, name), 1)
                else:
                    # Check if normalized
                    if "normalize" in struct and struct["normalize"]:
                        if struct["type"] == "Vector2D":
                            append_line("if(!this->{}.is_normalized()) {{".format(name), 1)
                            append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} is not a normal 2D vector (%f, %f) -> %f\", this->{}.i.read(), this->{}.j.read(), this->{}.calculate_scale());".format(struct_name, name, name, name, name), 2)
                        elif struct["type"] == "Vector3D":
                            append_line("if(!this->{}.is_normalized()) {{".format(name), 1)
                            append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} is not a normal 3D vector (%f, %f, %f) -> %f\", this->{}.i.read(), this->{}.j.read(), this->{}.k.read(), this->{}.calculate_scale());".format(struct_name, name, name, name, name, name), 2)
                        elif struct["type"] == "Quaternion":
                            append_line("if(!this->{}.is_normalized()) {{".format(name), 1)
                            append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} is not a normal quaternion (%f, %f, %f / %f) -> %f\", this->{}.i.read(), this->{}.j.read(), this->{}.k.read(), this->{}.w.read(), this->{}.calculate_scale());".format(struct_name, name, name, name, name, name, name), 2)
                        elif struct["type"] == "Plane2D":
                            append_line("if(!this->{}.vector.is_normalized()) {{".format(name), 1)
                            append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} is not a normal 2D plane (%f, %f) -> %f\", this->{}.vector.i.read(), this->{}.vector.j.read(), this->{}.vector.calculate_scale());".format(struct_name, name, name, name, name), 2)
                        elif struct["type"] == "Plane3D":
                            append_line("if(!this->{}.vector.is_normalized()) {{".format(name), 1)
                            append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} is not a normal 3D plane (%f, %f, %f) -> %f\", this->{}.vector.i.read(), this->{}.vector.j.read(), this->{}.vector.k.read(), this->{}.vector.calculate_scale());".format(struct_name, name, name, name, name, name), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                    
                    # Go through all enums
                    if struct["type"] in all_types and all_types[struct["type"]]["type"] == "enum":
                        e = all_types[struct["type"]]
                        shifted_by_one = "+ 1" if ("shifted_by_one" in struct and struct["shifted_by_one"]) else ""
                        append_line("if(!workload.disable_error_checking && static_cast<std::uint16_t>(this->{}{}) >= {}) {{".format(name, shifted_by_one, len(e["options"])), 1)
                        append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} is out of range (%zu >= {})\", static_cast<std::size_t>(this->{}{}));".format(struct_name, name, len(e["options"]), name, shifted_by_one), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                        
                        # Anything marked as excluded should be excluded
                        for o in e["options"]:
                            if "excluded" in o and o["excluded"]:
                                append_line("if(this->{} == {}) {{".format(struct["member_name"], o["member_name"]), 1)
                                append_line("REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, \"{}::{} has an invalid value (%zu)\", static_cast<std::size_t>(this->{}{}));".format(struct_name, name, name, shifted_by_one), 2)
                                append_line("throw InvalidTagDataException();", 2)
                                append_line("}", 1)
                                
                    # Make sure the value is within bounds
                    if minimum != None:
                        append_line("if(!workload.disable_error_checking && this->{} < {}) {{".format(name, minimum), 1)
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must be at least {}\", tag_index);".format(struct_name, name, minimum), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                    if maximum != None:
                        append_line("if(!workload.disable_error_checking && this->{} > {}) {{".format(name, maximum), 1)
                        append_line("workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must no more than {}\", tag_index);".format(struct_name, name, maximum), 2)
                        append_line("throw InvalidTagDataException();", 2)
                        append_line("}", 1)
                    append_line("r.{} = this->{};".format(name, name), 1)
            if "post_compile" in s and s["post_compile"]:
                append_line("this->post_compile(workload, tag_index, struct_index, offset);".format(struct_name, struct_name), 1)

            ## Remove our struct from the top of the stack
            append_line("stack->erase(stack->begin());", 1)
            append_line("}")
