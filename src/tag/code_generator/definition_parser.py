# SPDX-License-Identifier: GPL-3.0-only

import json
import os
import sys

compound_structs = [
    "ColorARGB",
    "ColorRGB",
    "Euler2D",
    "Euler3D",
    "Matrix",
    "Plane2D",
    "Plane3D",
    "Point2D",
    "Point2DInt",
    "Point3D",
    "Quaternion",
    "Rectangle2D",
    "TagDataOffset",
    "TagDependency",
    "TagReflexive",
    "Vector2D",
    "Vector3D"
]

def parse_definitions_from_dir(directory):
    # Read everything
    definitions = {}
    failed = False
    for f in os.scandir(directory):
        if f.is_file() and f.path.endswith(".json"):
            with open(f.path, "r") as file:
                try:
                    definitions[os.path.basename(f.path).split(".")[0]] = {
                        "definitions": json.loads(file.read()),
                        "dependencies": []
                    }
                except json.decoder.JSONDecodeError:
                    print("Failed to parse {}".format(f.path), file=sys.stderr)
                    failed = True

    # If we failed, exit
    if failed:
        sys.exit(1)

    # Go through every definition and find dependencies
    for name in definitions:
        definition_entry = definitions[name]
        for definition in definition_entry["definitions"]:
            if definition["type"] == "struct":
                def add_dependency_for_thing(thing):
                    for name_other in definitions:
                        if name_other != name and name_other not in definition_entry["dependencies"]:
                            for definition_other in definitions[name_other]["definitions"]:
                                if definition_other["name"] == thing:
                                    definition_entry["dependencies"].append(name_other)
                                    break
                
                for field in definition["fields"]:
                    add_dependency_for_thing(field["type"])
                    if field["type"] == "TagReflexive":
                        add_dependency_for_thing(field["struct"])
                    
                if "inherits" in definition:
                    add_dependency_for_thing(definition["inherits"])

    # Fix enum base
    def fix_enum_base(enum_base):
        if enum_base is not None:
            enum_fixed = ""
            for c in enum_base.replace("UI", "Ui").replace("GBX", "Gbx").replace("HUD", "Hud"):
                if enum_fixed != "" and c.isupper():
                    enum_fixed = enum_fixed + "_"
                enum_fixed = enum_fixed + c.upper()
            return enum_fixed

    # Set member/display names
    def add_names(o, enum_base = None):
        base_name = o["name"]
        enum_fixed = None
        
        if "member_name" not in o:
            member_name = base_name.lower().replace(" ", "_").replace("-", "_").replace("(","").replace(")", "").replace("'", "")
            
            # If it's an enum, append the name to it
            if enum_base is not None:
                member_name = enum_base + "_" + member_name.upper()
            
            # Prefix with underscore if it starts with a digit
            elif member_name[0].isdigit():
                member_name = "_" + member_name
            
            # Set that stuff
            o["member_name"] = member_name
            
        if "display_name" not in o:
            o["display_name"] = base_name
        
    c_list = []

    # Clean things up
    for name in definitions:
        definition_entry = definitions[name]
        for d in definition_entry["definitions"]:
            if d["type"] == "enum":
                new_options = []
                for o in d["options"]:
                    if isinstance(o, str):
                        new_options.append({
                            "name": o
                        })
                    else:
                        new_options.append(o)
                enum_base = fix_enum_base(d["name"])
                for o in new_options:
                    add_names(o, enum_base)
                d["enum_name"] = enum_base
                d["options"] = new_options
            elif d["type"] == "bitfield":
                new_fields = []
                for f in d["fields"]:
                    if isinstance(f, str):
                        new_fields.append({
                            "name": f
                        })
                    else:
                        new_fields.append(f)
                enum_base = fix_enum_base(d["name"] + "Flag")
                
                # Generate masks
                mask = 2**len(new_fields) - 1
                cache_only_mask = 0
                excluded_mask = 0
                read_only_mask = 0
                
                for i in range(0,len(new_fields)):
                    field = new_fields[i]
                    add_names(field, enum_base)
                    
                    bit = (1<<i)
                    
                    if "cache_only" in field and field["cache_only"]:
                        cache_only_mask = cache_only_mask | bit
                    
                    if "excluded" in field and field["excluded"]:
                        excluded_mask = excluded_mask | bit
                    
                    if "read_only" in field and field["read_only"]:
                        read_only_mask = read_only_mask | bit
                        
                d["cache_only_mask"] = cache_only_mask
                d["read_only_mask"] = read_only_mask
                d["mask"] = mask & ~excluded_mask
                    
                d["enum_name"] = enum_base
                d["fields"] = new_fields
            elif d["type"] == "struct":
                d["enum_name"] = None
                for f in d["fields"]:
                    if f["type"] != "pad":
                        add_names(f)
                        f["compound"] = f["type"] in compound_structs
    
    return definitions
