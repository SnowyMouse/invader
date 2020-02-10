# SPDX-License-Identifier: GPL-3.0-only

def make_definitions(f, ecpp, all_enums, all_bitfields, all_structs_arranged):
    f.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    header_name = "INVADER__TAG__HEK__CLASS__DEFINITION_HPP"
    f.write("#ifndef {}\n".format(header_name))
    f.write("#define {}\n\n".format(header_name))
    f.write("#include \"../../hek/data_type.hpp\"\n\n")
    f.write("namespace Invader::HEK {\n")

    ecpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    ecpp.write("#include <cstring>\n")
    ecpp.write("#include <cctype>\n")
    ecpp.write("#include <invader/tag/hek/definition.hpp>\n\n")
    ecpp.write("namespace Invader::HEK {\n")
    ecpp.write("    static inline bool equal_string_case_insensitive(const char *a, const char *b) noexcept {\n")
    ecpp.write("        while(std::tolower(*a) == std::tolower(*b)) {\n")
    ecpp.write("            if(*a == 0) {\n")
    ecpp.write("                return true;\n")
    ecpp.write("            }\n")
    ecpp.write("            a++; b++;\n")
    ecpp.write("        }\n")
    ecpp.write("        return false;\n")
    ecpp.write("    }\n")

    # Write enums at the top first, then bitfields
    for e in all_enums:
        f.write("    enum {} : TagEnum {{\n".format(e["name"]))

        # Convert PascalCase to UPPER_SNAKE_CASE
        prefix = ""
        name_to_consider = e["name"].replace("HUD", "Hud").replace("UI", "Ui")
        for i in name_to_consider:
            if prefix != "" and i.isupper():
                prefix += "_"
            prefix += i.upper()

        def format_enum(value):
            return "{}_{}".format(prefix,value.upper())

        def format_enum_str(value):
            return value.replace("_", "-")

        for n in range(0,len(e["options"])):
            f.write("        {},\n".format(format_enum(e["options"][n])))

        f.write("        {}\n".format(format_enum("enum_count")))

        f.write("    };\n")

        f.write("    /**\n")
        f.write("     * Get the string representation of the enum.\n")
        f.write("     * @param value value of the enum\n")
        f.write("     * @return      string representation of the enum\n")
        f.write("     */\n")
        f.write("    const char *{}_to_string({} value);\n".format(e["name"], e["name"]))
        ecpp.write("    const char *{}_to_string({} value) {{\n".format(e["name"], e["name"]))
        ecpp.write("        switch(value) {\n")
        for n in e["options"]:
            ecpp.write("        case {}::{}:\n".format(e["name"], format_enum(n)))
            ecpp.write("            return \"{}\";\n".format(format_enum_str(n)))
        ecpp.write("        default:\n")
        ecpp.write("            throw std::exception();\n")
        ecpp.write("        }\n")
        ecpp.write("    }\n")

        f.write("    /**\n")
        f.write("     * Get the enum value from the string.\n")
        f.write("     * @param value value of the enum as a string\n")
        f.write("     * @return      value of the enum\n")
        f.write("     */\n")
        f.write("    {} {}_from_string(const char *value);\n".format(e["name"], e["name"]))
        ecpp.write("    {} {}_from_string(const char *value) {{\n".format(e["name"], e["name"]))
        for n in range(0,len(e["options"])):
            ecpp.write("        {}if(equal_string_case_insensitive(value, \"{}\")) {{\n".format("" if n == 0 else "else ", format_enum_str(e["options"][n])))
            ecpp.write("             return {}::{};\n".format(e["name"], format_enum(e["options"][n])))
            ecpp.write("        }\n")
        ecpp.write("        else {\n")
        ecpp.write("            throw std::exception();\n")
        ecpp.write("        }\n")
        ecpp.write("    }\n")

    for b in all_bitfields:
        f.write("    struct {} {{\n".format(b["name"]))
        for q in b["fields"]:
            f.write("        std::uint{}_t {} : 1;\n".format(b["width"], q))
        f.write("    };\n")

    ecpp.write("}\n")

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

            name = n["name"]
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
                    f.write("            {}({});\n".format("COPY_THIS_ARRAY" if "count" in n else "COPY_THIS", n["name"]))
        f.write("            return copy;\n")
        f.write("        }\n")

        f.write("    };\n")
        f.write("    static_assert(sizeof({}<NativeEndian>) == 0x{:X});\n".format(s["name"], s["size"]))

    f.write("}\n\n")
    f.write("#endif\n")
