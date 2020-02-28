# SPDX-License-Identifier: GPL-3.0-only

def make_parser_struct(cpp_struct_value, all_used_structs, hpp, struct_name):
    hpp.write("        std::vector<ParserStructValue> get_values() override;\n".format(struct_name))
    cpp_struct_value.write("std::vector<ParserStructValue> {}::get_values() {{\n".format(struct_name))
    cpp_struct_value.write("    return std::vector<ParserStructValue>();\n")
    cpp_struct_value.write("}\n")
    pass
