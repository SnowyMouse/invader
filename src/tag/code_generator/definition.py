# SPDX-License-Identifier: GPL-3.0-only

def make_definitions(f, ecpp, bcpp, all_enums, all_bitfields, all_structs_arranged):
    f.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    header_name = "INVADER__TAG__HEK__CLASS__DEFINITION_HPP"
    f.write("#ifndef {}\n".format(header_name))
    f.write("#define {}\n\n".format(header_name))
    f.write("#include \"../../hek/data_type.hpp\"\n\n")
    f.write("namespace Invader::HEK {\n")

    ecpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    ecpp.write("#include <cstring>\n")
    ecpp.write("#include <invader/printf.hpp>\n")
    ecpp.write("#include <invader/tag/hek/definition.hpp>\n\n")
    ecpp.write("namespace Invader::HEK {\n")

    bcpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    bcpp.write("#include <cstring>\n")
    bcpp.write("#include <cctype>\n")
    bcpp.write("#include <invader/printf.hpp>\n")
    bcpp.write("#include <invader/tag/hek/definition.hpp>\n\n")
    bcpp.write("namespace Invader::HEK {\n")

    # Convert PascalCase to UPPER_SNAKE_CASE
    def format_enum(prefix, value):
        return "{}_{}".format(prefix,value.upper())

    def format_enum_str(value):
        return value.replace("_", "-").lower()

    def write_enum(name, fields, fields_pretty, type, cpp):
        f.write("    enum {} : {} {{\n".format(name, type))
        prefix = ""
        name_to_consider = name.replace("HUD", "Hud").replace("UI", "Ui").replace("GBX", "Gbx")
        for i in name_to_consider:
            if prefix != "" and i.isupper():
                prefix += "_"
            prefix += i.upper()
        for n in range(0,len(fields)):
            f.write("        {}{},\n".format(format_enum(prefix, fields[n]), " = static_cast<{}>(1) << {}".format(type, n) if (cpp == bcpp) else ""))
        f.write("        {}\n".format(format_enum(prefix, "enum_count")))
        f.write("    };\n")

        f.write("    /**\n")
        f.write("     * Get the string representation of the enum.\n")
        f.write("     * @param value value of the enum\n")
        f.write("     * @return      string representation of the enum\n")
        f.write("     */\n")
        f.write("    const char *{}_to_string({} value);\n".format(name, name))
        cpp.write("    const char *{}_to_string({} value) {{\n".format(name, name))
        cpp.write("        switch(value) {\n")
        for n in fields:
            cpp.write("        case {}::{}:\n".format(name, format_enum(prefix, n)))
            cpp.write("            return \"{}\";\n".format(format_enum_str(n)))
        cpp.write("        default:\n")
        cpp.write("            throw std::exception();\n")
        cpp.write("        }\n")
        cpp.write("    }\n")

        f.write("    /**\n")
        f.write("     * Get the pretty string representation of the enum.\n")
        f.write("     * @param value value of the enum\n")
        f.write("     * @return      pretty string representation of the enum\n")
        f.write("     */\n")
        f.write("    const char *{}_to_string_pretty({} value);\n".format(name, name))
        cpp.write("    const char *{}_to_string_pretty({} value) {{\n".format(name, name))
        cpp.write("        switch(value) {\n")
        for n in range(0,len(fields)):
            cpp.write("        case {}::{}:\n".format(name, format_enum(prefix, fields[n])))
            cpp.write("            return \"{}\";\n".format(fields_pretty[n]))
        cpp.write("        default:\n")
        cpp.write("            throw std::exception();\n")
        cpp.write("        }\n")
        cpp.write("    }\n")

        f.write("    /**\n")
        f.write("     * Get the enum value from the string.\n")
        f.write("     * @param value value of the enum as a string\n")
        f.write("     * @return      value of the enum\n")
        f.write("     */\n")
        f.write("    {} {}_from_string(const char *value);\n".format(name, name))
        cpp.write("    {} {}_from_string(const char *value) {{\n".format(name, name))
        for n in range(0,len(fields)):
            cpp.write("        {}if(std::strcmp(value, \"{}\") == 0) {{\n".format("" if n == 0 else "else ", format_enum_str(fields[n])))
            cpp.write("             return {}::{};\n".format(name, format_enum(prefix, fields[n])))
            cpp.write("        }\n")
        cpp.write("        else {\n")
        cpp.write("            throw std::exception();\n")
        cpp.write("        }\n")
        cpp.write("    }\n")
        f.write("\n")


    # Write enums at the top first, then bitfields
    for e in all_enums:
        write_enum(e["name"], e["options_formatted"], e["options"], "TagEnum", ecpp)
        
    for b in all_bitfields:
        f.write("    using {} = std::uint{}_t;\n".format(b["name"], b["width"]))
        write_enum("{}Flag".format(b["name"]), b["fields_formatted"], b["fields"], "std::uint{}_t".format(b["width"]), bcpp)

    ecpp.write("}\n")
    bcpp.write("}\n")

    # Now the hard part
    padding_present = False

    for s in all_structs_arranged:
        f.write("    ENDIAN_TEMPLATE(EndianType) struct {} {}{{\n".format(s["name"], ": {}<EndianType> ".format(s["inherits"]) if "inherits" in s else ""))
        for n in s["fields"]:
            type_to_write = n["type"]

            if type_to_write.startswith("int") or type_to_write.startswith("uint"):
                type_to_write = "std::{}_t".format(type_to_write)
            elif type_to_write == "pad":
                f.write("        PAD(0x{:X});\n".format(n["size"]))
                continue

            if "flagged" in n and n["flagged"]:
                type_to_write = "FlaggedInt<{}>".format(type_to_write)

            name = n["member_name"]
            if "count" in n:
                name = "{}[{}]".format(name, n["count"])

            format_to_use = None

            default_endian = "EndianType"
            if "endian" in n:
                if n["endian"] == "little":
                    default_endian = "LittleEndian"
                elif n["endian"] == "big":
                    default_endian = "BigEndian"
                elif n["endian"] == None:
                    default_endian = None

            if type_to_write == "TagReflexive":
                f.write("        TagReflexive<{}, {}> {};\n".format(default_endian, n["struct"], name))
            else:
                if default_endian is None:
                    format_to_use = "{}"
                elif "compound" in n and n["compound"]:
                    format_to_use = "{{}}<{}>".format(default_endian)
                else:
                    format_to_use = "{}<{{}}>".format(default_endian)

                if "bounds" in n and n["bounds"]:
                    f.write("        Bounds<{}> {};\n".format(format_to_use.format(type_to_write), name))
                else:
                    f.write("        {} {};\n".format(format_to_use.format(type_to_write), name))

        # Make sure we have all of the structs we depend on, too
        depended_structs = []
        dependency = s
        padding_present = False

        while dependency is not None:
            depended_structs.append(dependency)
            if not padding_present:
                for n in dependency["fields"]:
                    if n["type"] == "pad":
                        padding_present = True
                        break
            if "inherits" in dependency:
                dependency_name = dependency["inherits"]
                dependency = None
                for ds in all_structs_arranged:
                    if ds["name"] == dependency_name:
                        dependency = ds
                        break
            else:
                break

        # And we can't forget the copy part
        f.write("        ENDIAN_TEMPLATE(NewEndian) operator {}<NewEndian>() const {{\n".format(s["name"]))
        f.write("            {}<NewEndian> copy{};\n".format(s["name"], " = {}" if padding_present else ""))

        for ds in depended_structs:
            for n in ds["fields"]:
                if n["type"] == "pad":
                    continue
                else:
                    f.write("            {}({});\n".format("COPY_THIS_ARRAY" if "count" in n else "COPY_THIS", n["member_name"]))
        f.write("            return copy;\n")
        f.write("        }\n")

        f.write("    };\n")
        f.write("    static_assert(sizeof({}<NativeEndian>) == 0x{:X});\n\n".format(s["name"], s["size"]))

    f.write("}\n\n")
    f.write("#endif\n")
