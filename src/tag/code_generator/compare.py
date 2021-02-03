# SPDX-License-Identifier: GPL-3.0-only

def make_compare(all_used_structs, struct_name, all_bitfields, hpp, cpp_compare):
    hpp.write("        bool compare(const ParserStruct *what, bool precision, bool ignore_volatile, bool verbose, std::size_t depth) const override;\n")
    cpp_compare.write("    bool {}::compare(const ParserStruct *what, [[maybe_unused]] bool precision, [[maybe_unused]] bool ignore_volatile, [[maybe_unused]] bool verbose, [[maybe_unused]] std::size_t depth) const {{\n".format(struct_name))
    cpp_compare.write("        auto *what_value = dynamic_cast<const {} *>(what);\n".format(struct_name))
    cpp_compare.write("        auto result = true;\n".format(struct_name))
    cpp_compare.write("        if(!what_value) {\n")
    cpp_compare.write("            return false;\n")
    cpp_compare.write("        }\n")
    for struct in all_used_structs:
        if "cache_only" in struct and struct["cache_only"]:
            continue
        name = struct["member_name"]
        volatile = "volatile" in struct and struct["volatile"]
        
        def write_float_check(what):
            cpp_compare.write("        if({}this->{} != what_value->{} && (!precision || std::fabs(this->{}) - std::fabs(what_value->{}) > 0.000001)) {{\n".format("!ignore_volatile && " if volatile else "", what,what,what,what))
            cpp_compare.write("            if(verbose) {\n")
            cpp_compare.write("                for(std::size_t k = 0; k < depth * 2; k++) { oprintf(\" \"); }\n")
            cpp_compare.write("                oprintf_success_warn(\"{}::{} is different\");\n".format(struct_name, name))
            cpp_compare.write("            }\n")
            cpp_compare.write("            else {\n")
            cpp_compare.write("                return false;\n")
            cpp_compare.write("            }\n")
            cpp_compare.write("            result = false;\n");
            cpp_compare.write("        }\n")
            
        def write_regular_check(what):
            cpp_compare.write("        if({}this->{} != what_value->{}) {{\n".format("!ignore_volatile && " if volatile else "", what,what))
            cpp_compare.write("            if(verbose) {\n")
            cpp_compare.write("                for(std::size_t k = 0; k < depth * 2; k++) { oprintf(\" \"); }\n")
            cpp_compare.write("                oprintf_success_warn(\"{}::{} is different\");\n".format(struct_name, name))
            cpp_compare.write("            }\n")
            cpp_compare.write("            else {\n")
            cpp_compare.write("                return false;\n")
            cpp_compare.write("            }\n")
            cpp_compare.write("            result = false;\n");
            cpp_compare.write("        }\n")
             
        def write_memcmp_check(what):
            cpp_compare.write("        if({}std::memcmp(&this->{}, &what_value->{}, sizeof(this->{})) != 0) {{\n".format("!ignore_volatile && " if volatile else "", what,what,what))
            cpp_compare.write("            if(verbose) {\n")
            cpp_compare.write("                for(std::size_t k = 0; k < depth * 2; k++) { oprintf(\" \"); }\n")
            cpp_compare.write("                oprintf_success_warn(\"{}::{} is different\");\n".format(struct_name, name))
            cpp_compare.write("            }\n")
            cpp_compare.write("            else {\n")
            cpp_compare.write("                return false;\n")
            cpp_compare.write("            }\n")
            cpp_compare.write("            result = false;\n");
            cpp_compare.write("        }\n")
        
        if "count" in struct:
            continue
        
        if struct["type"] == "TagReflexive":
            cpp_compare.write("        auto this_{}_count = this->{}.size();\n".format(name,name))
            cpp_compare.write("        auto other_{}_count = what_value->{}.size();\n".format(name,name))
            cpp_compare.write("        if(this_{}_count != other_{}_count) {{\n".format(name,name))
            cpp_compare.write("            result = false;\n")
            cpp_compare.write("        }\n")
            cpp_compare.write("        else {")
            cpp_compare.write("            for(std::size_t i = 0; i < this_{}_count; i++) {{\n".format(name))
            cpp_compare.write("                if(!this->{}[i].compare(&what_value->{}[i], precision, ignore_volatile, verbose, depth + 1)) {{\n".format(name, name))
            cpp_compare.write("                    if(verbose) {\n")
            cpp_compare.write("                        for(std::size_t k = 0; k < depth * 2; k++) { oprintf(\" \"); }\n")
            cpp_compare.write("                        oprintf_success_warn(\"{}::{}#%zu is different\", i);\n".format(struct_name, name))
            cpp_compare.write("                    }\n")
            cpp_compare.write("                    else {\n")
            cpp_compare.write("                        return false;\n")
            cpp_compare.write("                    }\n")
            cpp_compare.write("                    result = false;\n")
            cpp_compare.write("                }\n")
            cpp_compare.write("            }\n")
            cpp_compare.write("        }\n")
            
        elif struct["type"] == "Vector2D":
            write_float_check("{}.i".format(name))
            write_float_check("{}.j".format(name))
        elif struct["type"] == "Vector3D":
            write_float_check("{}.i".format(name))
            write_float_check("{}.j".format(name))
            write_float_check("{}.k".format(name))
        elif struct["type"] == "Quaternion":
            write_float_check("{}.i".format(name))
            write_float_check("{}.j".format(name))
            write_float_check("{}.k".format(name))
            write_float_check("{}.w".format(name))
        elif struct["type"] == "Plane2D":
            write_float_check("{}.vector.i".format(name))
            write_float_check("{}.vector.j".format(name))
            write_float_check("{}.w".format(name))
        elif struct["type"] == "Plane3D":
            write_float_check("{}.vector.i".format(name))
            write_float_check("{}.vector.j".format(name))
            write_float_check("{}.vector.k".format(name))
            write_float_check("{}.w".format(name))
        elif struct["type"] == "Point2D":
            write_float_check("{}.x".format(name))
            write_float_check("{}.y".format(name))
        elif struct["type"] == "Point2DInt":
            write_regular_check("{}.x".format(name))
            write_regular_check("{}.y".format(name))
        elif struct["type"] == "Point3D":
            write_float_check("{}.x".format(name))
            write_float_check("{}.y".format(name))
            write_float_check("{}.z".format(name))
        elif struct["type"] == "ColorRGB":
            write_float_check("{}.red".format(name))
            write_float_check("{}.green".format(name))
            write_float_check("{}.blue".format(name))
        elif struct["type"] == "ColorARGB":
            write_float_check("{}.alpha".format(name))
            write_float_check("{}.red".format(name))
            write_float_check("{}.green".format(name))
            write_float_check("{}.blue".format(name))
        elif struct["type"] == "ColorARGBInt":
            write_float_check("{}.alpha".format(name))
            write_float_check("{}.red".format(name))
            write_float_check("{}.green".format(name))
            write_float_check("{}.blue".format(name))
        elif struct["type"] == "Euler3D":
            write_float_check("{}.yaw".format(name))
            write_float_check("{}.pitch".format(name))
            write_float_check("{}.roll".format(name))
        elif struct["type"] == "Euler2D":
            write_float_check("{}.yaw".format(name))
            write_float_check("{}.pitch".format(name))
        elif struct["type"] == "Rectangle2D":
            write_regular_check("{}.top".format(name))
            write_regular_check("{}.left".format(name))
            write_regular_check("{}.bottom".format(name))
            write_regular_check("{}.right".format(name))
        elif struct["type"] == "Matrix":
            for x in range(0,3):
                for y in range(0,3):
                    write_float_check("{}.matrix[{}][{}].read()".format(name,x,y))
        else:
            is_bitfield = False
            for i in all_bitfields:
                if i["name"] == struct["type"]:
                    is_bitfield = True
                    break
            
            if is_bitfield:
                write_memcmp_check(name)
            else:
                writer = write_float_check if struct["type"] == "Fraction" or struct["type"] == "Angle" or struct["type"] == "float" or struct["type"] == "double" else write_regular_check
                if ("bounds" in struct) and struct["bounds"]:
                    writer("{}.from".format(name))
                    writer("{}.to".format(name))
                else:
                    writer(name)
        
    cpp_compare.write("        return result;\n")
    cpp_compare.write("    }\n")
