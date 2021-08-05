# SPDX-License-Identifier: GPL-3.0-only

def make_parser_struct_functions(definition, append_line):
    for s in definition["definitions"]:
        if s["type"] == "struct":
            struct_name = s["name"]
            
            append_line("const char *{}::struct_name() const noexcept {{".format(struct_name))
            append_line("return \"{}\";".format(struct_name), 1)
            append_line("}")
            
            # Title (if set)
            if "title" in s:
                struct_title = s["title"]
                append_line("bool {}::has_title() const noexcept {{".format(struct_name))
                append_line("return true;", 1)
                append_line("}")
                append_line("const char *{}::title() const noexcept {{".format(struct_name))
                for f in s["fields"]:
                    if f["type"] == "pad":
                        continue
                    if "name" in f and f["name"] == struct_title:
                        if f["type"] == "TagString":
                            append_line("return this->{}.string;".format(f["member_name"]), 1)
                        elif f["type"] == "TagDependency":
                            append_line("const auto *start = this->{}.path.c_str();".format(f["member_name"]), 1)
                            append_line("for(const char *q = start; q && *q; q++) {", 1)
                            append_line("if(*q == '\\\\') {", 2)
                            append_line("start = q + 1;", 3)
                            append_line("}", 2)
                            append_line("}", 1)
                            append_line("return start;", 1)
                        else:
                            raise Exception("ohno")
                append_line("}")

