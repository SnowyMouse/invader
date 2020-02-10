# SPDX-License-Identifier: GPL-3.0-only

def make_cache_deformat(post_cache_deformat, all_used_structs, struct_name, hpp, cpp_cache_deformat_data):
    hpp.write("        void cache_deformat() override;\n")
    cpp_cache_deformat_data.write("    void {}::cache_deformat() {{\n".format(struct_name))
    cpp_cache_deformat_data.write("        if(this->cache_formatted) {\n")
    for struct in all_used_structs:
        if struct["type"] == "TagReflexive":
            cpp_cache_deformat_data.write("            for(auto &i : {}) {{\n".format(struct["name"]))
            cpp_cache_deformat_data.write("                i.cache_deformat();\n")
            cpp_cache_deformat_data.write("            }\n")
    cpp_cache_deformat_data.write("            this->cache_formatted = false;\n")
    if post_cache_deformat:
        cpp_cache_deformat_data.write("            this->post_cache_deformat();\n")
    cpp_cache_deformat_data.write("        }\n")
    cpp_cache_deformat_data.write("    }\n")
