# SPDX-License-Identifier: GPL-3.0-only

import os

# Headers!
def generate_headers(definitions, header_dir):
    # One header to rule them all (import everything - very slow but may be useful for a catch-all routine)
    everything_hpp = []
    everything_hpp.append("// This code was auto-generated. Changes made will be lost.")
    everything_hpp.append("")
    everything_hpp.append("#ifndef INVADER__TAG__PARSER__DEFINITION__ALL_HPP")
    everything_hpp.append("#define INVADER__TAG__PARSER__DEFINITION__ALL_HPP")
    for filename in definitions:
        everything_hpp.append("#include \"{}.hpp\"".format(filename))
    everything_hpp.append("#endif")
    with open(os.path.join(header_dir, "all.hpp"), "w") as f:
        for n in everything_hpp:
            f.write(n + "\n")
    
    # Now, without further ado, generate the headers
    for filename in definitions:
        definition = definitions[filename]
        
        # Build the header file
        hpp = []
        indent = 0
        
        def append_line(what = None):
            if what is None:
                hpp.append("")
            else:
                hpp.append("    " * indent + what)
        
        # Inclusion guards
        append_line("// This code was auto-generated. Changes made will be lost.")
        append_line()
        append_line("#ifndef INVADER__TAG__PARSER__DEFINITION__{}_HPP".format(filename.upper()))
        append_line("#define INVADER__TAG__PARSER__DEFINITION__{}_HPP".format(filename.upper()))
        
        append_line()
        append_line("#include <cstdint>")
        append_line("#include <optional>")
        append_line("#include <vector>")
        append_line("#include <cstddef>")
        append_line("#include <invader/hek/data_type.hpp>")
        append_line("#include <invader/tag/parser/parser_struct.hpp>")
        
        append_line()
        
        # Add some more headers
        for i in definition["dependencies"]:
            append_line("#include \"{}.hpp\"".format(i))
        
        # Namespace
        append_line()
        append_line("namespace Invader::Parser {")
        
        indent = indent + 1
        
        # First add bitfields and enums
        for i in definition["definitions"]:
            if i["type"] == "enum":
                append_line("enum {} : TagEnum {{".format(i["name"]))
                indent = indent + 1
                for f in range(0, len(i["options"])):
                    append_line("{},".format(i["options"][f]["member_name"]))
                append_line("{}_ENUM_COUNT".format(i["enum_name"]))
                indent = indent - 1
                append_line("};")
                append_line("std::optional<{}> {}_from_string(const char *) noexcept;".format(i["name"], i["name"]))
                append_line("const char *{}_to_string({}) noexcept;".format(i["name"], i["name"]))
                append_line("const char *{}_to_string_pretty({}) noexcept;".format(i["name"], i["name"]))
                append_line()
            elif i["type"] == "bitfield":
                append_line("enum {}Flag : std::uint{}_t {{".format(i["name"], i["width"]))
                indent = indent + 1
                for f in range(0, len(i["fields"])):
                    append_line("{} = {}{}".format(i["fields"][f]["member_name"], 2 ** f, "," if f + 1 < len(i["fields"]) else ""))
                indent = indent - 1
                append_line("};")
                append_line("std::optional<{}Flag> {}Flag_from_string(const char *) noexcept;".format(i["name"], i["name"]))
                append_line("const char *{}Flag_to_string({}Flag) noexcept;".format(i["name"], i["name"]))
                append_line("const char *{}Flag_to_string_pretty({}Flag) noexcept;".format(i["name"], i["name"]))
                append_line("using {} = std::uint{}_t;".format(i["name"], i["width"]))
                append_line()
        
        # Next add structs
        for i in definition["definitions"]:
            if i["type"] == "struct":
                append_line("struct {} : {} {{".format(i["name"], i["inherits"] if "inherits" in i else "ParserStruct"))
                indent = indent + 1
                
                def add_fields(use_parser_struct):
                    # Add fields
                    for f in i["fields"]:
                        if f["type"] == "pad":
                            if not use_parser_struct:
                                append_line("std::byte __{}_pad_{}[{}];".format(i["name"], len(hpp), f["size"]))
                            continue
                        type = f["type"]
                        
                        # Handle arrays here
                        if type == "TagReflexive":
                            if use_parser_struct:
                                type = "std::vector<{}>".format(f["struct"])
                            else:
                                type = "TagReflexive<Endianness, {}::C>".format(f["struct"])
                        # Dependencies
                        elif use_parser_struct and type == "TagDependency":
                            type = "ParsedTagDependency"
                        # Raw data
                        elif use_parser_struct and type == "TagDataOffset":
                            type = "std::vector<std::byte>"
                        # Other types
                        else:
                            if type.startswith("int") or type.startswith("uint"):
                                type = "std::{}_t".format(type)
                                
                            endianness = "NativeEndian"
                            if not use_parser_struct:
                                if "endian" not in f:
                                    endianness = "Endianness"
                                elif f["endian"] == None:
                                    endianness = None
                                elif f["endian"] == "little":
                                    endianness = "LittleEndian"
                                elif f["endian"] == "big":
                                    endianness = "BigEndian"
                            
                            if "flagged" in f and f["flagged"]:
                                type = "FlaggedInt<{}>".format(type)
                                
                            if f["compound"]:
                                type = "{}<{}>".format(type, endianness)
                            elif endianness is not None and not use_parser_struct:
                                type = "{}<{}>".format(endianness, type)
                            
                            if "bounds" in f and f["bounds"]:
                                type = "Bounds<{}>".format(type)
                        mname = f["member_name"]
                        if "count" in f:
                            mname = "{}[{}]".format(mname, f["count"])
                        append_line("{} {};".format(type, mname))
                        
                append_line("template <template<typename> class Endianness> struct C {}{{".format(": " + i["inherits"] + "::C<Endianness> " if "inherits" in i else ""))
                indent = indent + 1
                add_fields(False)
                
                append_line("template <template<typename> class NewEndianness> operator C<NewEndianness>() const {")
                indent = indent + 1
                append_line("C<NewEndianness> copy = {};")
                
                # Copy operator
                if "inherits" in i:
                    append_line("*reinterpret_cast<{}::C<NewEndianness> *>(&copy) = *reinterpret_cast<{}::C<Endianness> *>(this);".format(i["inherits"], i["inherits"]))
                for f in i["fields"]:
                    if f["type"] != "pad":
                        if "count" in f:
                            append_line("std::copy(this->{}, this->{} + {}, copy.{});".format(f["member_name"], f["member_name"], f["count"], f["member_name"]))
                        else:
                            append_line("copy.{} = this->{};".format(f["member_name"], f["member_name"]))
                append_line("return copy;")
                
                indent = indent - 1
                append_line("}")
                
                
                indent = indent - 1
                
                append_line("};")
                append_line("static_assert(sizeof(C<NativeEndian>) == {});".format(i["size"]))
                append_line()
                
                add_fields(True)
                
                append_line()
                append_line("bool check_for_invalid_indices(bool) override;")
                append_line("bool check_for_invalid_indices(bool, std::deque<std::tuple<const ParserStruct *, std::size_t, const char *>> &) override;")
                append_line("bool check_for_nonnormal_vectors(bool) override;")
                append_line("bool check_for_invalid_ranges(bool) override;")
                append_line("void cache_deformat() override;")
                append_line("std::size_t refactor_reference(const char *, TagFourCC, const char *, TagFourCC) override;")
                append_line("const char *struct_name() const noexcept override;")
                append_line("std::vector<ParserStructValue> get_values_internal() override;")
                
                append_line("void compile(BuildWorkload &, std::size_t, std::size_t, std::optional<std::size_t> = std::nullopt, std::size_t = 0, std::deque<const ParserStruct *> * = nullptr) override;")
                append_line("std::vector<std::byte> generate_hek_tag_data(bool = false) override;")
                append_line("~{}() override = default;".format(i["name"]))
                append_line()
                append_line("static {} parse_hek_tag_file(const std::byte *data, std::size_t data_size, bool postprocess = false);".format(i["name"]))
                append_line("static {} parse_cache_file_data(const Invader::Tag &tag, std::optional<Pointer> pointer = std::nullopt);".format(i["name"]))
                append_line("static {} parse_hek_tag_data(const std::byte *data, std::size_t data_size, std::size_t &data_read, bool postprocess = false, const C<BigEndian> *data_this = nullptr);".format(i["name"]))
                append_line("static void scan_padding(const Invader::Tag &tag, std::optional<Pointer> pointer = std::nullopt);")
                
                def append_if_true(text, what):
                    if what in i and i[what]:
                        append_line(text)
                        
                # Add any functions needed
                append_if_true("void postprocess_hek_data();", "postprocess_hek_data")
                append_if_true("void post_cache_deformat();", "post_cache_deformat")
                append_if_true("void post_cache_parse(const Invader::Tag &, std::optional<Pointer>);", "post_cache_parse")
                append_if_true("void pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);", "pre_compile")
                append_if_true("void post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);", "post_compile")
                append_if_true("bool check_for_nonnormal_vectors_more(bool normalize);", "normalize")
                append_if_true("bool has_title() const noexcept override;", "title")
                append_if_true("const char *title() const noexcept override;", "title")
                
                indent = indent - 1
                        
                append_line("};")
                append_line()
                pass
            
        indent = indent - 1
        
        # All done
        append_line("}")
        
        # Done
        append_line("#endif")

        # Output the stuff
        with open(os.path.join(header_dir, filename + ".hpp"), "w") as f:
            for n in hpp:
                f.write(n + "\n")
