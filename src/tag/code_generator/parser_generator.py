# SPDX-License-Identifier: GPL-3.0-only

import os
import parser_generator_compile
import parser_generator_parse_cache_file_data
import parser_generator_parse_hek_tag_file
import parser_generator_parse_hek_tag_data
import parser_generator_parser_struct

# Generate the parser
def generate_parser(definitions, parser_dir):
    # Find all bitfields and enums
    all_types = {}
    
    for filename in definitions:
        for d in definitions[filename]["definitions"]:
            all_types[d["name"]] = d
    
    for filename in definitions:
        definition = definitions[filename]
        
        # Build the C++ file
        cpp = []
        indent = 0
        
        # Appendafy the line
        def append_line(what = None, indent_delta = 0):
            if what is None:
                cpp.append("")
            else:
                cpp.append("    " * (indent + indent_delta) + what)
                
        append_line("// This code was auto-generated. Changes made will be lost.")
        append_line()
        append_line("#include <invader/tag/parser/definition/{}.hpp>".format(filename))
        append_line("#include <invader/file/file.hpp>")
        append_line("#include <invader/map/tag.hpp>")
        append_line("#include <invader/map/map.hpp>")
        append_line("#include <invader/tag/hek/header.hpp>")
        append_line("#include <invader/build/build_workload.hpp>")
        if filename == "model":
            append_line("#include <invader/tag/parser/definition/gbxmodel.hpp>")
            
        append_line()
        append_line("using namespace Invader::File;")
        append_line()
        append_line("namespace Invader::Parser {")
        indent = indent + 1
        parser_generator_parse_cache_file_data.make_parse_cache_file_data(definition, all_types, append_line)
        parser_generator_compile.make_compile(definition, all_types, append_line)
        parser_generator_parse_hek_tag_file.make_parse_hek_tag_file(definition, append_line)
        parser_generator_parse_hek_tag_data.make_parse_hek_tag_data(definition, all_types, append_line)
        parser_generator_parser_struct.make_parser_struct_functions(definition, append_line)
        indent = indent - 1
        append_line("}")
        
        # Write the stuff
        with open(os.path.join(parser_dir, filename + ".cpp"), "w") as f:
            for n in cpp:
                f.write(n + "\n")
