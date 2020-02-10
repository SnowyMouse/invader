# SPDX-License-Identifier: GPL-3.0-only

def make_cache_format_data(struct_name, s, pre_compile, post_compile, all_used_structs, hpp, cpp_cache_format_data):
    # compile()
    hpp.write("        void compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp = std::nullopt, std::size_t offset = 0) override;\n")
    cpp_cache_format_data.write("    void {}::compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp, std::size_t offset) {{\n".format(struct_name))
    cpp_cache_format_data.write("        auto *start = workload.structs[struct_index].data.data();\n")
    cpp_cache_format_data.write("        workload.structs[struct_index].bsp = bsp;\n")
    cpp_cache_format_data.write("        workload.structs[struct_index].unsafe_to_dedupe = {};\n".format("true" if ("unsafe_to_dedupe" in s and s["unsafe_to_dedupe"]) else "false"))
    if pre_compile:
        cpp_cache_format_data.write("        if(!this->cache_formatted) {\n")
        cpp_cache_format_data.write("            this->pre_compile(workload, tag_index, struct_index, offset);\n")
        cpp_cache_format_data.write("        }\n")
        cpp_cache_format_data.write("        this->cache_formatted = true;\n")
    cpp_cache_format_data.write("        auto &r = *reinterpret_cast<struct_little *>(start + offset + tag_index * 0);\n")
    cpp_cache_format_data.write("        std::fill(reinterpret_cast<std::byte *>(&r), reinterpret_cast<std::byte *>(&r), std::byte());\n")
    for struct in all_used_structs:
        if ("non_cached" in struct and struct["non_cached"]) or ("compile_ignore" in struct and struct["compile_ignore"]):
            continue
        name = struct["name"]
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
            if "minimum" in struct:
                minimum = struct["minimum"]
                cpp_cache_format_data.write("        if(t_{}_count < {}) {{\n".format(name, minimum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must have at least {} block{}\", tag_index);\n".format(struct_name, name, minimum, "" if minimum == 1 else "s"))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
            if "maximum" in struct:
                maximum = struct["maximum"]
                cpp_cache_format_data.write("        if(t_{}_count > {}) {{\n".format(name, maximum))
                cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} must have no more than {} block{}\", tag_index);\n".format(struct_name, name, maximum, "" if maximum == 1 else "s"))
                cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
                cpp_cache_format_data.write("        }\n")
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
            cpp_cache_format_data.write("                    this->{}[i].compile(workload, tag_index, p.struct_index, bsp, i * STRUCT_SIZE);\n".format(name))
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
            cpp_cache_format_data.write("        r.{}.from = this->{}.from;\n".format(name, name))
            cpp_cache_format_data.write("        r.{}.to = this->{}.to;\n".format(name, name))
        elif "count" in struct and struct["count"] > 1:
            cpp_cache_format_data.write("        std::copy(this->{}, this->{} + {}, r.{});\n".format(name, name, struct["count"], name))
        elif struct["type"] == "enum":
            cpp_cache_format_data.write("        if(static_cast<std::uint16_t>(r.{}) >= {}) {{\n".format(name, len(struct["options"])))
            cpp_cache_format_data.write("            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, \"{}::{} exceeds maximum value of {}\", tag_index);\n".format(struct_name, name, len(struct["options"])))
            cpp_cache_format_data.write("            throw InvalidTagDataException();\n")
            cpp_cache_format_data.write("        }\n")
            cpp_cache_format_data.write("        r.{} = this->{};\n".format(name, name))
        else:
            cpp_cache_format_data.write("        r.{} = this->{};\n".format(name, name))
    if post_compile:
        cpp_cache_format_data.write("        this->post_compile(workload, tag_index, struct_index, offset);\n".format(name, name))
    cpp_cache_format_data.write("    }\n")
