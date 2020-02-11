# SPDX-License-Identifier: GPL-3.0-only

def make_refactor_reference(all_used_structs, struct_name, hpp, cpp_refactor_reference):
    hpp.write("\n        /**\n")
    hpp.write("         * Refactor the tag reference, replacing all references with the given reference. Paths must use Halo path separators.\n")
    hpp.write("         * @param from_path  Path to look for\n")
    hpp.write("         * @param from_class Class to look for\n")
    hpp.write("         * @param to_path    Path to replace with\n")
    hpp.write("         * @param to_class   Class to replace with\n")
    hpp.write("         * @return           number of references replaced\n")
    hpp.write("         */\n")
    hpp.write("        std::size_t refactor_reference(const char *from_path, TagClassInt from_class, const char *to_path, TagClassInt to_class) override;\n".format(struct_name))
    cpp_refactor_reference.write("    std::size_t {}::refactor_reference([[maybe_unused]] const char *from_path, [[maybe_unused]] TagClassInt from_class, [[maybe_unused]] const char *to_path, [[maybe_unused]] TagClassInt to_class) {{\n".format(struct_name))
    cpp_refactor_reference.write("        std::size_t replaced = 0;\n")
    for struct in all_used_structs:
        name = struct["name"]
        if struct["type"] == "TagDependency":
            cpp_refactor_reference.write("        if(this->{}.tag_class_int == from_class && this->{}.path == from_path) {{\n".format(name, name))

            # Make sure the class is correct for the reference
            if struct["classes"][0] != "*":
                test_line = ""
                error_line = ""
                classes = struct["classes"]
                classes_len = len(classes)
                for c in range(0, classes_len):
                    if c != 0:
                        test_line = " && " + test_line
                    test_line = "to_class != TagClassInt::TAG_CLASS_{}".format(classes[c].upper()) + test_line
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

                cpp_refactor_reference.write("            if({}) {{\n".format(test_line))
                cpp_refactor_reference.write("                eprintf_error(\"{}::{} can only be{}\");\n".format(struct_name, name, error_line))
                cpp_refactor_reference.write("                throw InvalidTagDataException();\n")
                cpp_refactor_reference.write("            }\n")

            cpp_refactor_reference.write("            this->{}.tag_class_int = to_class;\n".format(name))
            cpp_refactor_reference.write("            this->{}.path = to_path;\n".format(name))
            cpp_refactor_reference.write("            replaced++;\n")
            cpp_refactor_reference.write("        }\n")
        elif struct["type"] == "TagReflexive":
            cpp_refactor_reference.write("        for(auto &i : this->{}) {{\n".format(name))
            cpp_refactor_reference.write("            replaced += i.refactor_reference(from_path, from_class, to_path, to_class);\n")
            cpp_refactor_reference.write("        }\n")
    cpp_refactor_reference.write("        return replaced;\n")
    cpp_refactor_reference.write("    }\n")
