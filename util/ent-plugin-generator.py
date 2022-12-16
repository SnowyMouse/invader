# SPDX-License-Identifier: GPL-3.0-only

import sys
import json
import os

if len(sys.argv) < 2:
    print("Usage: {} <json> [json [...]]".format(sys.argv[0]), file=sys.stderr)
    sys.exit(1)

tag_structs = {
    'Actor': 'actr',
    'ActorVariant': 'actv',
    'Antenna': 'ant!',
    'ModelAnimations': 'antr',
    'Biped': 'bipd',
    'Bitmap': 'bitm',
    'Spheroid': 'boom',
    'ContinuousDamageEffect': 'cdmg',
    'ModelCollisionGeometry': 'coll',
    'ColorTable': 'colo',
    'Contrail': 'cont',
    'DeviceControl': 'ctrl',
    'Decal': 'deca',
    'UIWidgetDefinition': 'DeLa',
    'InputDeviceDefaults': 'devc',
    'Device': 'devi',
    'DetailObjectCollection': 'dobc',
    'Effect': 'effe',
    'Equipment': 'eqip',
    'Flag': 'flag',
    'Fog': 'fog ',
    'Font': 'font',
    'MaterialEffects': 'foot',
    'Garbage': 'garb',
    'Glow': 'glw!',
    'GrenadeHUDInterface': 'grhi',
    'HUDMessageText': 'hmt ',
    'HUDNumber': 'hud#',
    'HUDGlobals': 'hudg',
    'Item': 'item',
    'ItemCollection': 'itmc',
    'DamageEffect': 'jpt!',
    'LensFlare': 'lens',
    'Lightning': 'elec',
    'DeviceLightFixture': 'lifi',
    'Light': 'ligh',
    'SoundLooping': 'lsnd',
    'DeviceMachine': 'mach',
    'Globals': 'matg',
    'Meter': 'metr',
    'LightVolume': 'mgs2',
    'GBXModel': 'mod2',
    'Model': 'mode',
    'MultiplayerScenarioDescription': 'mply',
    'PreferencesNetworkGame': 'ngpr',
    'Object': 'obje',
    'Particle': 'part',
    'ParticleSystem': 'pctl',
    'Physics': 'phys',
    'Placeholder': 'plac',
    'PointPhysics': 'pphy',
    'Projectile': 'proj',
    'WeatherParticleSystem': 'rain',
    'ScenarioStructureBSP': 'sbsp',
    'Scenery': 'scen',
    'ShaderTransparentChicagoExtended': 'scex',
    'ShaderTransparentChicago': 'schi',
    'Scenario': 'scnr',
    'ShaderEnvironment': 'senv',
    'ShaderTransparentGlass': 'sgla',
    'Shader': 'shdr',
    'Sky': 'sky ',
    'ShaderTransparentMeter': 'smet',
    'Sound': 'snd!',
    'SoundEnvironment': 'snde',
    'ShaderModel': 'soso',
    'ShaderTransparentGeneric': 'sotr',
    'UIWidgetCollection': 'Soul',
    'ShaderTransparentPlasma': 'spla',
    'SoundScenery': 'ssce',
    'StringList': 'str#',
    'ShaderTransparentWater': 'swat',
    'TagCollection': 'tagc',
    'CameraTrack': 'trak',
    'Dialogue': 'udlg',
    'UnitHUDInterface': 'unhi',
    'Unit': 'unit',
    'UnicodeStringList': 'ustr',
    'VirtualKeyboard': 'vcky',
    'Vehicle': 'vehi',
    'Weapon': 'weap',
    'Wind': 'wind',
    'WeaponHUDInterface': 'wphi'
}

files = []
all_enums = []
all_bitfields = []
all_structs = []

for i in range(1, len(sys.argv)):
    objects = None
    with open(sys.argv[i], "r") as f:
        objects = json.loads(f.read())
    name = os.path.basename(sys.argv[i]).split(".")[0]
    files.append(name)

    # Get all enums, bitfields, and structs
    for s in objects:
        if s["type"] == "enum":
            all_enums.append(s)
        elif s["type"] == "bitfield":
            all_bitfields.append(s)
        elif s["type"] == "struct":
            all_structs.append(s)
        else:
            print("Unknown object type {}".format(s["type"]), file=sys.stderr)
            sys.exit(1)

def to_hex(number):
    return "0x{:X}".format(number)

for s in all_structs:
    if s["name"] in tag_structs:
        with open("{}.ent".format(tag_structs[s["name"]]), "w") as f:
            def write_struct(struct, offset = 0, indent = 0):
                initial_offset = offset
                def write(text):
                    f.write("{}{}\n".format(" " * indent * 4, text))
                def write_basic_entity(type, name):
                    write("<{} name=\"{}\" offset=\"{}\" visible=\"true\" />".format(type, name, to_hex(offset)))

                if "inherits" in struct and struct["inherits"]:
                    found_it = False
                    for i in all_structs:
                        if i["name"] == struct["inherits"]:
                            found_it = True
                            offset += write_struct(i, offset, indent)
                    if not found_it:
                        print("Failed to find {}".format(s["inherits"]))
                        sys.exit()
                for field in struct["fields"]:
                    type = field["type"]
                    if type == "pad":
                        offset += field["size"]
                        continue

                    count = 1 if "count" not in field else field["count"]
                    bounds = False if "bounds" not in field or field["bounds"] == False else True

                    for c in range(count):
                        field_name = (field["name"] if count == 1 else "{} ({})".format(field["name"], c)).title().replace(" A ", " a ").replace(" An ", " an ").replace(" The ", " the ")
                        for b in range(2):
                            name = field_name
                            if bounds:
                                if b == 0:
                                    name = "{}: From".format(field_name)
                                else:
                                    name = "{}: To".format(field_name)
                            elif b == 1:
                                break

                            if "endian" in field and field["endian"] == "little":
                                name = "{} (CALCULATED)".format(name)

                            if type == "TagString":
                                write_basic_entity("string32", name)
                                offset += 0x20
                            elif type == "ModelAnimationArrayNodeTransformFlagData" or type == "ModelAnimationArrayNodeRotationFlagData" or type == "ModelAnimationArrayNodeScaleFlagData":
                                offset += 4
                            elif type == "ScenarioType":
                                offset += 2
                            elif type == "TagDependency":
                                write_basic_entity("dependency", name)
                                offset += 0x10
                            elif type == "float" or type == "Angle" or type == "Fraction":
                                write_basic_entity("float", name)
                                offset += 0x4
                            elif type == "TagFourCC":
                                write_basic_entity("string4", name)
                                offset += 0x4
                            elif type == "Vector2D":
                                write_basic_entity("float", "{}: I".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: J".format(name))
                                offset += 0x4
                            elif type == "Vector3D":
                                write_basic_entity("float", "{}: I".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: J".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: K".format(name))
                                offset += 0x4
                            elif type == "Plane2D":
                                write_basic_entity("float", "{}: I".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: J".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: W".format(name))
                                offset += 0x4
                            elif type == "ScenarioStructureBSPArrayVertex":
                                write_basic_entity("int16", "{}".format(name))
                                offset += 0x2
                            elif type == "ScenarioStructureBSPArrayVertexBuffer":
                                offset += 0x4
                                write_basic_entity("int32", "{}: Count".format(name))
                                offset += 0x4
                                write_basic_entity("int32", "{}: Offset".format(name))
                                offset += 0x4
                                offset += 0x8
                            elif type == "Quaternion" or type == "Plane3D":
                                write_basic_entity("float", "{}: I".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: J".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: K".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: W".format(name))
                                offset += 0x4
                            elif type == "Rectangle2D":
                                write_basic_entity("int16", "{}: Top".format(name))
                                offset += 0x2
                                write_basic_entity("int16", "{}: Right".format(name))
                                offset += 0x2
                                write_basic_entity("int16", "{}: Bottom".format(name))
                                offset += 0x2
                                write_basic_entity("int16", "{}: Left".format(name))
                                offset += 0x2
                            elif type == "Matrix":
                                write_basic_entity("float", "{}: X1".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Y1".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Z1".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: X2".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Y2".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Z2".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: X3".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Y3".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Z3".format(name))
                                offset += 0x4
                            elif type == "Point3D":
                                write_basic_entity("float", "{}: X".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Y".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Z".format(name))
                                offset += 0x4
                            elif type == "Point2D":
                                write_basic_entity("float", "{}: X".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Y".format(name))
                                offset += 0x4
                            elif type == "Point2DInt":
                                write_basic_entity("int16", "{}: X".format(name))
                                offset += 0x2
                                write_basic_entity("int16", "{}: Y".format(name))
                                offset += 0x2
                            elif type == "TagDataOffset":
                                write_basic_entity("int32", "{}: size".format(name))
                                offset += 0x4
                                write_basic_entity("int32", "{}: external (CALCULATED)".format(name))
                                offset += 0x4
                                write_basic_entity("int32", "{}: offset (CALCULATED)".format(name))
                                offset += 0x4
                                write_basic_entity("int32", "{}: pointer (CALCULATED)".format(name))
                                offset += 0x8
                            elif type == "Euler2D":
                                write_basic_entity("float", "{}: Yaw".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Pitch".format(name))
                                offset += 0x4
                            elif type == "Euler3D":
                                write_basic_entity("float", "{}: Yaw".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Pitch".format(name))
                                offset += 0x4
                                write_basic_entity("float", "{}: Roll".format(name))
                                offset += 0x4
                            elif type == "ColorRGB":
                                write_basic_entity("colorRGB", name)
                                offset += 0xC
                            elif type == "ColorARGB":
                                write_basic_entity("colorARGB", name)
                                offset += 0x10
                            elif type == "TagID":
                                write_basic_entity("loneID", name)
                                offset += 0x4
                            elif type == "ColorARGBInt":
                                write_basic_entity("colorbyte", name)
                                offset += 0x4
                            elif type == "Pointer":
                                write_basic_entity("int32", name)
                                offset += 0x4
                            elif type.startswith("int"):
                                write_basic_entity(type, name)
                                offset += int(type[3:]) // 8
                            elif type.startswith("uint"):
                                write_basic_entity(type[1:], name)
                                offset += int(type[4:]) // 8
                            elif type.startswith("TagClassInt"):
                                write_basic_entity("string4", name)
                                offset += 4
                            elif type.startswith("Index"):
                                write_basic_entity("int16", name)
                                offset += 2
                            elif type == "TagReflexive":
                                found_it = False
                                for r in all_structs:
                                    if r["name"] == field["struct"]:
                                        write("<struct name=\"{}\" offset=\"{}\" visible=\"true\" size=\"{}\">".format(name, to_hex(offset), r["size"]))
                                        write_struct(r, 0, indent + 1)
                                        write("</struct>")
                                        found_it = True
                                        break
                                if found_it == False:
                                    if field["struct"] != "PredictedResource":
                                        print("Failed to find {}".format(field["struct"]))
                                        sys.exit()
                                write_basic_entity("int32", "{}: array size".format(name))
                                offset += 0x4
                                write_basic_entity("int32", "{}: pointer (CALCULATED)".format(name))
                                offset += 0x4
                                offset += 0x4
                            else:
                                found_it = False

                                # Is it a bitfield?
                                for bitfield in all_bitfields:
                                    if bitfield["name"] == type:
                                        width = bitfield["width"]
                                        write("<bitmask{} name=\"{}\" offset=\"{}\" visible=\"true\">".format(width, name, to_hex(offset)))
                                        bit = width
                                        indent += 1
                                        for field in bitfield["fields"]:
                                            bit -= 1
                                            write("<option name=\"{}\" value=\"{}\" />".format(field, bit))
                                        indent -= 1
                                        write("</bitmask{}>".format(width))
                                        found_it = True
                                        offset += width // 8
                                        break
                                if found_it:
                                    continue

                                # Is it an enum?
                                for enum in all_enums:
                                    if enum["name"] == type:
                                        write("<enum16 name=\"{}\" offset=\"{}\" visible=\"true\">".format(name, to_hex(offset)))
                                        value = 0
                                        indent += 1
                                        for field in enum["options"]:
                                            write("<option name=\"{}\" value=\"{}\" />".format(field, value))
                                            value += 1
                                        indent -= 1
                                        write("</enum16>")
                                        found_it = True
                                        offset += 2
                                        break

                                if found_it:
                                    continue

                                # Don't know then?
                                print("Unknown type {}".format(type))
                                sys.exit(1)

                if offset - initial_offset != struct["size"]:
                    print("Size doesn't add up for {}: 0x{:X} (expected) != 0x{:X} (gotten)".format(struct["name"], struct["size"], offset - initial_offset))
                    #sys.exit(1)
                return offset

            f.write("<plugin class=\"{}\" author=\"Aerocatia, Snowy Mouse\" version=\"1.0\" headersize=\"{}\">\n".format(tag_structs[s["name"]], s["size"]))
            write_struct(s, 24 if s["name"] == "ScenarioStructureBSP" else 0, 1)
            f.write("</plugin>\n")
