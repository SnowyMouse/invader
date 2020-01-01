# SPDX-License-Identifier: GPL-3.0-only

import sys
import json
import os

all_enums = []
all_bitfields = []
all_structs = []

for i in range(1, len(sys.argv)):
    print(sys.argv[i])

    lines = []
    with open(sys.argv[i], "r") as f:
        lines = [x.strip("\n") for x in f.readlines()]

    l = 0
    max_l = len(lines)

    enums = []
    bitfields = []
    structs = []

    while l < max_l:
        line = lines[l]
        if line.startswith("    "):
            line = line[4:]
            #print(line)

            s = line.split(" ")
            if len(s) >= 3:
                # add an enum
                if s[0] == "enum" and len(s) == 5 and s[3] == "TagEnum":
                    enum_field = {}
                    enum_field["name"] = s[1]
                    enum_field["options"] = []
                    while True:
                        l = l + 1
                        line = lines[l][4:]
                        if line == "};":
                            break
                        else:
                            line = line[4:].replace(",","")
                            if line == "" or line.isspace() or line.startswith("/*") or line.startswith("//"):
                                continue
                            enum_field["options"].append(line.split(" ")[0])
                    enum_field["type"] = "enum"
                    enums.append(enum_field)

                # Add some flags?
                elif s[0] == "struct" and len(s) == 3:
                    bitfield_field = {}
                    bitfield_field["name"] = s[1]
                    bitfield_field["type"] = "bitfield"
                    bitfield_field["fields"] = []
                    while True:
                        l = l + 1
                        line = lines[l][4:]
                        if line == "};":
                            break
                        line = line[4:]
                        line_split = line.split(" ")
                        if len(line_split) != 4 or line_split[3] != "1;":
                            print("Warning: Bad bitfield struct on line {}".format(l))
                            break
                        if "width" not in bitfield_field:
                            bitfield_field["width"] = int(line_split[0][9:-2])
                        bitfield_field["fields"].append(line_split[1])
                    bitfields.append(bitfield_field)

                # Add a single dependency struct
                elif s[0].startswith("SINGLE_DEPENDENCY_"):
                    padded_struct = s[0].startswith("SINGLE_DEPENDENCY_PADDED_STRUCT")

                    struct_field = {}
                    struct_field["name"] = s[0][len("SINGLE_DEPENDENCY_STRUCT(") if not padded_struct else len("SINGLE_DEPENDENCY_PADDED_STRUCT("):-1]
                    struct_field["type"] = "struct"

                    first_type = 4 if padded_struct else 3

                    # Get the dependency going
                    f = {}
                    f["name"] = s[1][:-1 if padded_struct else -2]
                    f["type"] = "TagDependency"
                    f["compound"] = True
                    f["classes"] = [s[i].replace(",", "") for i in range(first_type, len(s))]
                    struct_field["fields"] = [f]

                    # If it's a padded struct, add padding
                    if padded_struct:
                        p = {}
                        p["type"] = "pad"
                        p["size"] = int(s[2][:-2], 16)
                        struct_field["fields"].append(p)
                        # Size is 16 + padding
                        struct_field["size"] = 16 + p["size"]

                    # Size is 16 (size of a dependency)
                    else:
                        struct_field["size"] = 16

                    structs.append(struct_field)

                # Add a tag struct
                elif s[0] == "ENDIAN_TEMPLATE(EndianType)":
                    struct_field = {}
                    struct_field["name"] = s[2]
                    struct_field["fields"] = []
                    struct_field["type"] = "struct"

                    if len(s) == 6:
                        struct_field["inherits"] = s[4][:-len("<EndianType>")]

                    struct_fields_text = []
                    while True:
                        l = l + 1
                        line = lines[l][4:]
                        if line == "};":
                            break

                        line = line[4:]
                        if line.startswith("ENDIAN_TEMPLATE"):
                            break

                        if line.isspace() or line.startswith("//") or line.startswith("/**") or line == "":
                            continue
                        else:
                            struct_fields_text.append(line)

                    for s in struct_fields_text:
                        if s.startswith("PAD"):
                            f = {}
                            f["type"] = "pad"
                            f["size"] = int(s[4:-2], 16)
                            struct_field["fields"].append(f)
                        else:
                            s_split = s.split(" ")
                            f = {}
                            if s.startswith("TagReflexive"):
                                f["name"] = s_split[2][:-1]
                                f["type"] = "TagReflexive"
                                f["struct"] = s_split[1][:-1]
                                f["compound"] = True

                            else:
                                f["name"] = s_split[1][:-1]
                                type_r = s_split[0]

                                if type_r.startswith("Bounds"):
                                    f["bounds"] = True
                                    type_r = type_r[len("Bounds<"):-1]

                                # Check if it starts with the endian (thus 4 bytes or less). If it's little endian, we should also mark it as such so it remains hidden.
                                if type_r.startswith("EndianType") or type_r.startswith("LittleEndian"):
                                    is_little = type_r.startswith("LittleEndian")
                                    type_read = type_r[len("LittleEndian<"):-1] if is_little else type_r[len("EndianType<"):-1]

                                    if type_read.startswith("FlaggedInt<"):
                                        type_read = type_read[11:-1]
                                        f["flagged"] = True
                                    if type_read.startswith("std::"):
                                        type_read = type_read[5:-2]
                                    f["type"] = type_read

                                    if is_little:
                                        f["endian"] = "little"

                                # Check if it ends with it then. Again, if it's little endian, we should mark it as such
                                elif type_r.endswith("<EndianType>") or type_r.endswith("<LittleEndian>"):
                                    is_little = type_r.endswith("<LittleEndian>")
                                    type_read = type_r[:-len("<LittleEndian>")] if is_little else type_r[:-len("<EndianType>")]

                                    if type_read == "TagDependency":
                                        f["type"] = "TagDependency"
                                        f["classes"] = []
                                        f["compound"] = True
                                        if len(s_split) > 3:
                                            for q in range(3, len(s_split)):
                                                class_to_add = s_split[q]
                                                if len(class_to_add) < 3:
                                                    continue
                                                if class_to_add.startswith("."):
                                                    class_to_add = class_to_add[1:]
                                                if class_to_add.endswith(","):
                                                    class_to_add = class_to_add[:-1]
                                                f["classes"].append(class_to_add)
                                    else:
                                        f["type"] = type_read


                                    if is_little:
                                        f["endian"] = "little"

                                    f["compound"] = True

                                elif type_r.startswith("std::"):
                                    f["type"] = type_r[5:-2]
                                    f["endian"] = None

                                else:
                                    f["type"] = type_r
                                    f["endian"] = None

                                # If we have an array, include that
                                if f["name"].endswith("]"):
                                    array_index = f["name"].find("[")
                                    f["count"] = int(f["name"][array_index:][1:-1])
                                    f["name"] = f["name"][:array_index]

                            new_name = f["name"].replace("_", " ").replace(" t ","'t ").replace("dont","don't").replace("wont","won't").replace("cant","can't")
                            if new_name == " ":
                                new_name = f["name"][1:]

                            f["name"] = new_name

                            struct_field["fields"].append(f)

                    # Add it
                    structs.append(struct_field)

        l = l + 1

    # Last but not least, let's set the sizes for every struct
    for line in lines:
        if line.startswith("    "):
            line = line[4:]
            if line.startswith("static_assert(sizeof("):
                line_split = line.split(" ")
                check_split = "<NativeEndian>)" if line_split[0].endswith("<NativeEndian>)") else "<BigEndian>)"
                if len(line_split) != 3 or not line_split[0].endswith(check_split):
                    print("Warning: Invalid static_assert(sizeof(...)): {}".format(line))
                    continue
                struct_size = int(line_split[2][:-2],16)
                struct_name = line_split[0][len("static_assert(sizeof("):-len(check_split)]

                for s in structs:
                    if s["name"] == struct_name:
                        s["size"] = struct_size

    for s in structs:
        if "size" not in s:
            print("Warning: Struct {} is missing a size".format(s["name"]))

    final_json_data = []

    # Clean up the enums
    for e in enums:
        name = e["name"]
        name_in_snake_case = ""
        name = name.replace("HUD", "Hud").replace("UI", "Ui")
        for n in name:
            if n.isupper() and name_in_snake_case != "":
                name_in_snake_case += "_" + n
            else:
                name_in_snake_case += n.upper()
        name_in_snake_case += "_"

        requires_manual = False
        for q in e["options"]:
            if not q.startswith(name_in_snake_case):
                print("Warning: Enum {} will need its strings to be manually fixed".format(e["name"]))
                requires_manual = True
                break

        if not requires_manual:
            for q in range(0, len(e["options"])):
                f = e["options"][q][len(name_in_snake_case):].replace("_", " ").lower().replace("  ", " ")
                if f.startswith(" "):
                    f = f[1:]
                e["options"][q] = f

    # Clean up the bitfields
    for b in bitfields:
        for q in range(0, len(b["fields"])):
            f = b["fields"][q].replace("_", " ").replace(" t ", "'t ").replace("  ", " ")
            if f.startswith(" "):
                f = f[1:]
            if f == " 50 strength":
                print("no")
            b["fields"][q] = f

    all_enums.extend(enums)
    all_bitfields.extend(bitfields)
    all_structs.extend(structs)
    final_json_data.extend(enums)
    final_json_data.extend(bitfields)
    final_json_data.extend(structs)

    final_json = json.dumps(final_json_data, indent=4)

    with open(os.path.basename(sys.argv[i]).split(".")[0] + ".json", "w") as f:
        f.write("{}\n".format(final_json))

for e in all_enums:
    for e2 in all_enums:
        if e is e2 or len(e["options"]) != len(e2["options"]):
            continue
        different = False
        for n in range(0, len(e["options"])):
            if e["options"][n] != e2["options"][n]:
                different = True
                break
        if not different:
            print("Warning: Enum {} duplicates {}".format(e["name"], e2["name"]))
            break

for e in all_bitfields:
    for e2 in all_bitfields:
        if e is e2 or len(e["fields"]) != len(e2["fields"]) or e["width"] != e2["width"]:
            continue
        different = False
        for n in range(0, len(e["fields"])):
            if e["fields"][n] != e2["fields"][n]:
                different = True
                break
        if not different:
            print("Warning: Bitfield {} duplicates {}".format(e["name"], e2["name"]))
            break

# Get all the value types used
unique_structs_list = []
unique_types = []
for s in all_structs:
    if s["name"] not in unique_types:
        unique_structs_list.append(s["name"])
    for q in s["fields"]:
        if q["type"] not in unique_types:
            unique_types.append(q["type"])

# Remove stuff we know exists
for e in all_bitfields:
    if e["name"] in unique_types:
        unique_types.remove(e["name"])
for e in all_enums:
    if e["name"] in unique_types:
        unique_types.remove(e["name"])
for s in unique_structs_list:
    if s in unique_types:
        unique_types.remove(s)

print("Unreferenced types: {}".format(unique_types))
