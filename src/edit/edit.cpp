// SPDX-License-Identifier: GPL-3.0-only

#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/header.hpp>
#include <string>

#ifdef __linux__
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

enum ActionType {
    ACTION_TYPE_GET,
    ACTION_TYPE_SET,
    ACTION_TYPE_COUNT,
    ACTION_TYPE_LIST,
    ACTION_TYPE_INSERT,
    ACTION_TYPE_COPY,
    ACTION_TYPE_DELETE,
    ACTION_TYPE_MOVE
};

struct Actions {
    ActionType type;
    std::string key;
    std::string value;
    std::size_t count = 0;
    std::size_t position = 0;
};

static std::string get_top_member_name(const std::string &key, std::string &after_member) {
    auto *key_str = key.c_str();
    auto *c = key_str;
    for(; *c != 0 && *c != '.' && *c != '[' && *c != ']'; c++);
    
    if(c == key_str) {
        eprintf_error("Invalid key %s", key.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    auto return_value = std::string(key_str, c);
    after_member = std::string(c);
    return return_value;
}

static std::pair<std::size_t, std::size_t> get_range(const std::string &key, std::string &after_range) {
    auto *key_str = key.c_str();
    auto *key_end = key_str;
    
    if(key_str[0] == 0) {
        eprintf_error("Expected range at end of key");
        std::exit(EXIT_FAILURE);
    }
    
    if(key_str[0] != '[') {
        eprintf_error("Invalid range in key %s", key.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    for(; *key_end != ']'; key_end++) {
        // Unexpected end?
        if(*key_end == 0) {
            eprintf_error("Invalid range in key %s", key.c_str());
            std::exit(EXIT_FAILURE);
        }
    }
    
    auto range_str = std::string(key_str + 1, key_end);
    after_range = key_end + 1;
    
    // All of 'em
    if(range_str == "*") {
        return { 0, SIZE_MAX };
    }
    else if(range_str == "end") {
        return { SIZE_MAX, SIZE_MAX };
    }
    
    std::size_t min = 0, max = 0;
    std::size_t hyphens = 0;
    
    // Find hyphens
    for(auto &c : range_str) {
        if(c == '-') {
            hyphens++;
        }
    }
    
    // Okay
    if(hyphens > 1) {
        eprintf_error("Invalid range %s", range_str.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    if(hyphens == 0) {
        min = std::strtoul(range_str.c_str(), nullptr, 10);
        max = min;
    }
    else {
        try {
            std::size_t v;
            
            if(std::strncmp(range_str.c_str(), "end-", 4) == 0) {
                min = SIZE_MAX;
                v = 3;
            }
            else {
                min = std::stoul(range_str.c_str(), &v, 10);
            }
            
            v++;
            
            if(std::strcmp(range_str.c_str() + v, "end") == 0) {
                max = SIZE_MAX;
            }
            else {
                max = std::stoul(range_str.c_str() + v, nullptr, 10);
            }
        }
        catch (std::exception &) {
            eprintf_error("Invalid range %s", range_str.c_str());
        }
    }
    
    // Did we exceed things?
    if(min > max) {
        eprintf_error("Invalid range %s", range_str.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    return { min, max };
}

static void build_array(Invader::Parser::ParserStruct *ps, std::string key, std::vector<Invader::Parser::ParserStructValue> &array, std::string *bitfield, std::pair<std::size_t, std::size_t> *range) {
    if(key == "") {
        eprintf_error("Expected value name");
        std::exit(EXIT_FAILURE);
    }
    
    if(key[0] == '.') {
        key = std::string(key.begin() + 1, key.end());
    }
    else {
        eprintf_error("Expected a dot before key %s", key.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    auto member = get_top_member_name(key, key);
    if(member == "") {
        eprintf_error("No member name given for array");
        std::exit(EXIT_FAILURE);
    }
    
    auto values = ps->get_values();
    
    // Do it!
    for(auto &i : values) {
        auto *member_name = i.get_member_name();
        
        if(member_name && member_name == member) {
            if(i.get_type() == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
                // End of key?
                if(key.size() == 0) {
                    array.emplace_back(i);
                    return;
                }
                
                auto count = i.get_array_size();
                
                if(count == 0) {
                    eprintf_error("%s::%s is empty", ps->struct_name(), member.c_str());
                }
                
                auto access_range = get_range(key, key);
                
                if(access_range.first == SIZE_MAX) {
                    access_range.first = count - 1;
                }
                
                if(access_range.second == SIZE_MAX) {
                    access_range.second = count - 1;
                }
                
                if(count < access_range.first || count <= access_range.second) {
                    eprintf_error("%zu-%zu is out of bounds for %s::%s (%zu element%s)", access_range.first, access_range.second, ps->struct_name(), member.c_str(), count, count == 1 ? "" : "s");
                    std::exit(EXIT_FAILURE);
                }
                
                // Are we returning a range?
                if(key.size() == 0 && range) {
                    *range = access_range;
                    array.emplace_back(i);
                    return;
                }
                
                for(std::size_t k = access_range.first; k <= access_range.second; k++) {
                    build_array(&i.get_object_in_array(k), key, array, bitfield, range);
                }
                
                return;
            }
            else {
                array.emplace_back(i);
                
                // Is this a bitfield? If so, set it!
                if(i.get_type() == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK) {
                    if(key.size() == 0) {
                        eprintf_error("Expected bitfield but got the end of the key");
                        std::exit(EXIT_FAILURE);
                    }
                    else if(key[0] != '.') {
                        eprintf_error("Expected bitfield but got %s", key.c_str());
                        std::exit(EXIT_FAILURE);
                    }
                    *bitfield = key.substr(1);
                }
                else if(key.size() != 0) {
                    eprintf_error("Expected end of key but got %s", key.c_str());
                    std::exit(EXIT_FAILURE);
                }
                
                return;
            }
        }
    }
    
    eprintf_error("%s::%s does not exist", ps->struct_name(), member.c_str());
    std::exit(EXIT_FAILURE);
}

static void require_writable_only(const std::vector<Invader::Parser::ParserStructValue> &values, bool writable_only) {
    if(writable_only) {
        for(auto &i : values) {
            if(i.is_read_only()) {
                eprintf_error("%s is read-only", i.get_member_name());
                std::exit(EXIT_FAILURE);
            }
        }
    }
}

static std::vector<Invader::Parser::ParserStructValue> get_values_for_key(Invader::Parser::ParserStruct *ps, std::string key, std::string &bitfield, bool writable_only) {
    std::vector<Invader::Parser::ParserStructValue> values;
    build_array(ps, key, values, &bitfield, nullptr);
    require_writable_only(values, writable_only);
    return values;
}

static std::vector<Invader::Parser::ParserStructValue> get_values_for_key(Invader::Parser::ParserStruct *ps, std::string key, std::pair<std::size_t, std::size_t> &range, bool writable_only) {
    std::vector<Invader::Parser::ParserStructValue> values;
    build_array(ps, key, values, nullptr, &range);
    require_writable_only(values, writable_only);
    return values;
}

static std::vector<Invader::Parser::ParserStructValue> get_values_for_key(Invader::Parser::ParserStruct *ps, std::string key, bool writable_only) {
    std::vector<Invader::Parser::ParserStructValue> values;
    build_array(ps, key, values, nullptr, nullptr);
    require_writable_only(values, writable_only);
    return values;
}

static std::string get_value(const Invader::Parser::ParserStructValue &value, const std::string &bitmask) {
    auto format = value.get_number_format();
    auto type = value.get_type();
    if(format == Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_NONE) {
        switch(type) {
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING:
                return value.read_string();
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY:
                return Invader::File::halo_path_to_preferred_path(value.get_dependency().path) + "." + Invader::HEK::tag_fourcc_to_extension(value.get_dependency().tag_fourcc);
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM:
                return value.read_enum();
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK:
                return std::to_string(value.read_bitfield(bitmask.c_str()) ? 1 : 0);
            default:
                eprintf_error("Unsupported value type for this operation");
                std::exit(EXIT_FAILURE);
        }
    }
    
    else {
        std::string str;
        auto values = value.get_values();
        
        switch(format) {
            case Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_INT:
                for(auto &i : values) {
                    if(str.size() > 0) {
                        str += " ";
                    }
                    str += std::to_string(std::get<std::int64_t>(i));
                }
                break;
            case Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT:
                for(auto &i : values) {
                    if(str.size() > 0) {
                        str += " ";
                    }
                    
                    double multiplier = 1.0;
                    if(type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ANGLE || type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_EULER2D ||type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_EULER3D) {
                        multiplier = 180.0 / HALO_PI;
                    }
                    
                    str += std::to_string(std::get<double>(i) * multiplier);
                }
                break;
            default:
                std::terminate();
        }
    
        return str;
    }
}

static void set_value(Invader::Parser::ParserStructValue &value, const std::string &new_value, const std::optional<std::string> bitfield = std::nullopt) {
    auto format = value.get_number_format();
    auto type = value.get_type();
    
    // Something special?
    if(format == Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_NONE) {
        switch(type) {
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING:
                if(new_value.size() > 31) {
                    eprintf_error("String exceeds maximum length (%zu > 31)", new_value.size());
                    std::exit(EXIT_FAILURE);
                }
                return value.set_string(new_value.c_str());
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY:
                try {
                    auto &dep = value.get_dependency();
                    auto new_path = Invader::File::split_tag_class_extension(Invader::File::preferred_path_to_halo_path(new_value)).value();
                    dep.path = new_path.path;
                    dep.tag_fourcc = new_path.fourcc;
                    return;
                }
                catch (std::exception &) {
                    eprintf_error("Invalid tag path %s", new_value.c_str());
                    std::exit(EXIT_FAILURE);
                }
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM:
                try {
                    return value.write_enum(new_value.c_str());
                }
                catch (std::exception &) {
                    eprintf_error("Invalid enum value %s", new_value.c_str());
                    std::exit(EXIT_FAILURE);
                }
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK:
                try {
                    auto new_value_int = std::stoi(new_value);
                    switch(new_value_int) {
                        case 0:
                            return value.write_bitfield(bitfield.value().c_str(), false);
                        case 1:
                            return value.write_bitfield(bitfield.value().c_str(), true);
                        default:
                            eprintf_error("Bitfields can only be set to 0 or 1");
                            std::exit(EXIT_FAILURE);
                    }
                }
                catch (std::exception &) {
                    eprintf_error("Invalid bitmask/value %s => %s", bitfield.value().c_str(), new_value.c_str());
                    std::exit(EXIT_FAILURE);
                }
            default:
                eprintf_error("Unsupported value type");
                std::exit(EXIT_FAILURE);
        }
    }
    
    // Basic numerical value?
    else {
        const char *start = new_value.c_str();
        char *c;
        std::vector<Invader::Parser::ParserStructValue::Number> values;
        while(*start) {
            auto *current_path = start;
            if(*start != '-' && *start != '.' && (*start < '0' || *start > '9')) {
                eprintf_error("Invalid input value %s", current_path);
                std::exit(EXIT_FAILURE);
            }
            switch(format) {
                case Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_INT:
                    values.emplace_back(std::strtol(current_path, &c, 10));
                    break;
                case Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT: {
                    double multiplier = 1.0;
                    if(type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ANGLE || type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_EULER2D ||type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_EULER3D) {
                        multiplier = HALO_PI / 180.0;
                    }
                    values.emplace_back(std::strtod(current_path, &c) * multiplier);
                    break;
                }
                default: std::terminate();
            }
            start = c;
            if(*start == ' ') {
                start++;
            }
            else if(*start == 0) {
                break;
            }
            else {
                eprintf_error("Invalid input value %s", new_value.c_str());
                std::exit(EXIT_FAILURE);
            }
        }
        
        if(values.size() == value.get_value_count()) {
            value.set_values(values);
            return;
        }
        else {
            eprintf_error("Incorrect number of inputs for the value (got %zu, expected %zu)", values.size(), value.get_value_count());
            std::exit(EXIT_FAILURE);
        }
    }
    
    eprintf_error("Unimplemented");
    std::exit(EXIT_FAILURE);
}

// Ensure a struct has at least one of everything in everything (for listing)
static Invader::Parser::ParserStruct &populate_struct(Invader::Parser::ParserStruct &ps) {
    for(auto &i : ps.get_values()) {
        if(i.get_type() == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
            i.insert_objects_in_array(0, 1);
            populate_struct(i.get_object_in_array(0));
        }
    }
    return ps;
}

struct TagDataListTreeElement {
    std::size_t level;
    std::string name;
    std::string type;
    
    TagDataListTreeElement(std::size_t level, const std::string &name, const std::string &type) : level(level), name(name), type(type) {}
};

static void list_everything(Invader::Parser::ParserStruct &ps, std::vector<TagDataListTreeElement> &output, bool with_values, std::size_t level = 0) {
    for(auto &i : ps.get_values()) {
        auto *mv = i.get_member_name();
        if(!mv) {
            continue;
        }
        
        auto type = i.get_type();
        std::string type_str;
        std::string name = mv;
        
        switch(type) {
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE:
                type_str = "array";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_INT8:
                type_str = "int8";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_INT16:
                type_str = "int16";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_INT32:
                type_str = "int32";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_UINT8:
                type_str = "uint8";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_UINT16:
                type_str = "uint16";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_UINT32:
                type_str = "uint32";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_FLOAT:
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_FRACTION:
                type_str = "float32";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ANGLE:
                type_str = "angle (float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_INDEX:
                type_str = "index";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_COLORARGB:
                type_str = "color (ARGB float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_COLORRGB:
                type_str = "color (RGB float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_COLORARGBINT:
                type_str = "color (A8R8G8B8)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_POINT2DINT:
                type_str = "point2d (integer)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_POINT2D:
                type_str = "point2d (float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_POINT3D:
                type_str = "point3d (float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_PLANE2D:
                type_str = "plane2d (float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_PLANE3D:
                type_str = "plane3d (float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_QUATERNION:
                type_str = "quaternion (float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY:
                type_str = "dependency";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGID:
                type_str = "bullshit";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING:
                type_str = "string";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGDATAOFFSET:
                type_str = "data";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM:
                type_str = "enum";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK:
                type_str = "bitmask";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_MATRIX:
                type_str = "matrix (float32)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_VECTOR2D:
                type_str = "vector2d (float)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_VECTOR3D:
                type_str = "vector3d (float)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_EULER2D:
                type_str = "euler2d (float)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_EULER3D:
                type_str = "euler3d (float)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_RECTANGLE2D:
                type_str = "rectangle2d (float)";
                break;
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_GROUP_START:
                type_str = "group";
                break;
        }
        
        if(i.is_bounds()) {
            type_str += " (bounds)";
        }
        
        if(type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
            if(with_values) {
                name = name + "[" + std::to_string(i.get_array_size()) + "]";
            }
            else {
                name += "[]";
            }
        }
            
        output.emplace_back(level, name, type_str);
        
        // If reflexive, list that stuff
        if(type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
            auto count = i.get_array_size();
            for(std::size_t m = 0; m < count; m++) {
                list_everything(i.get_object_in_array(m), output, with_values, level + 1);
            }
        }
        
        // Or if it's a bitmask, do that too
        else if(type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK) {
            for(auto &j : i.list_enum()) {
                output.emplace_back(level + 1, j, "bitfield");
            }
        }
        
        // Or if it's an enum, list the values
        else if(type == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM) {
            for(auto &j : i.list_enum()) {
                output.emplace_back(level + 1, j, "enum value");
            }
        }
    }
}

static void list_everything(Invader::Parser::ParserStruct &ps, std::vector<std::string> &output, bool with_values = false) {
    // Print the right column (description)
    std::size_t terminal_width = 80;
    
    // Resize based on console width
    #ifdef __linux__
    struct winsize w = {};
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        std::size_t new_width = static_cast<std::size_t>(w.ws_col);
        terminal_width = new_width;
    }
    #endif
    
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO w;
    if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &w)) {
        std::size_t new_width = w.srWindow.Right - w.srWindow.Left + 1;
        terminal_width = new_width;
    }
    #endif
    
    std::vector<TagDataListTreeElement> array;
    list_everything(ps, array, with_values);
    
    auto array_size = array.size();
    for(std::size_t ar = 0; ar < array_size; ar++) {
        auto &element = array[ar];
        
        // Build the indent
        std::string indent;
        
        for(std::size_t e = 0; e < element.level; e++) {
            // Check to see if we have something on the same level
            bool has_next_in_array = false;
            for(std::size_t ar2 = ar + 1; ar2 < array_size; ar2++) {
                auto next_thing = array[ar2].level;
                
                // We have another element of the same level
                if(next_thing == e + 1) {
                    has_next_in_array = true;
                    break;
                }
                
                // We don't
                else if(next_thing <= e) {
                    break;
                }
            }
            
            if(has_next_in_array) {
                if(e + 1 == element.level) {
                    indent = indent + " ├──";
                }
                else {
                    indent = indent + " │  ";
                }
            }
            else {
                if(e + 1 == element.level) {
                    indent = indent + " └──";
                }
                else {
                    indent = indent + "    ";
                }
            }
        }
        
        // Get the actual length of the string (UTF-8)
        int len = 0;
        auto string = indent + element.name;
        const char *s = string.c_str();
        while (*s) len += (*s++ & 0xC0) != 0x80;
        
        // If we can include the type, do it
        if(len + 1 + element.type.size() < terminal_width) {
            for(std::size_t q = len; q < terminal_width - element.type.size(); q++) {
                string += " ";
            }
            string += element.type;
        }
        
        output.emplace_back(string);
    }
}

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES
    
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the font data or tag file.");
    options.emplace_back("get", 'G', 1, "Get the value with the given key.", "<key>");
    options.emplace_back("set", 'S', 2, "Set the value at the given key to the given value.", "<key> <val>");
    options.emplace_back("count", 'C', 1, "Get the number of elements in the array at the given key.", "<key>");
    options.emplace_back("list", 'L', 0, "List all elements in a tag.");
    options.emplace_back("new", 'N', 0, "Create a new tag");
    
    options.emplace_back("output", 'o', 1, "Output the tag to a different path rather than overwriting it.", "<file>");
    options.emplace_back("save-as", 'O', 1, "Output the tag to a different path relative to the tags directory rather than overwriting it.", "<tag>");
    
    options.emplace_back("insert", 'I', 3, "Add # structs to the given index or \"end\" if the end of the array.", "<key> <#> <pos>");
    options.emplace_back("move", 'M', 2, "Swap the selected structs with the structs at the given index or \"end\" if the end of the array. The regions must not intersect.", "<key> <pos>");
    options.emplace_back("erase", 'E', 1, "Delete the selected struct(s).", "<key>");
    options.emplace_back("copy", 'c', 2, "Copy the selected struct(s) to the given index or \"end\" if the end of the array.", "<key> <pos>");

    static constexpr char DESCRIPTION[] = "Edit tags via command-line.";
    static constexpr char USAGE[] = "[options] <tag.class>";

    struct EditOptions {
        std::filesystem::path tags = "tags";
        bool use_filesystem_path = false;
        std::vector<Actions> actions;
        bool new_tag = false;
        bool check_read_only = true;
        std::optional<std::variant<std::string, std::filesystem::path>> overwrite_path;
    } edit_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<EditOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, edit_options, [](char opt, const std::vector<const char *> &arguments, auto &edit_options) {
        switch(opt) {
            case 't':
                edit_options.tags = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'P':
                edit_options.use_filesystem_path = true;
                break;
            case 'G':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_GET, arguments[0], {}, 0, 0 });
                break;
            case 'C':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_COUNT, arguments[0], {}, 0, 0 });
                break;
            case 'L':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_LIST, {}, {}, 0, 0 });
                break;
            case 'S':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_SET, arguments[0], arguments[1], 0, 0 });
                break;
            case 'I':
                try {
                    edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_INSERT, arguments[0], {}, std::stoul(arguments[1]), std::strcmp(arguments[2], "end") == 0 ? SIZE_MAX : std::stoul(arguments[2]) });
                }
                catch(std::exception &) {
                    eprintf_error("Expected a valid count/position");
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'M':
                try {
                    edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_MOVE, arguments[0], {}, 0, std::strcmp(arguments[1], "end") == 0 ? SIZE_MAX : std::stoul(arguments[1]) });
                }
                catch(std::exception &) {
                    eprintf_error("Expected a valid position");
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'E':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_DELETE, arguments[0], {}, 0, 0 });
                break;
            case 'N':
                edit_options.new_tag = true;
                break;
            case 'o':
                edit_options.overwrite_path = std::filesystem::path(arguments[0]);
                break;
            case 'O':
                edit_options.overwrite_path = std::string(arguments[0]);
                break;
            case 'c':
                try {
                    edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_COPY, arguments[0], {}, 0, std::strcmp(arguments[1], "end") == 0 ? SIZE_MAX : std::stoul(arguments[1]) });
                }
                catch(std::exception &) {
                    eprintf_error("Expected a valid position");
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });

    std::filesystem::path file_path;
    if(edit_options.use_filesystem_path) {
        file_path = std::string(remaining_arguments[0]);
    }
    else {
        file_path = std::filesystem::path(edit_options.tags) / Invader::File::halo_path_to_preferred_path(remaining_arguments[0]);
    }
    
    std::unique_ptr<Invader::Parser::ParserStruct> tag_struct;
    
    // Make a new tag... or don't
    Invader::HEK::TagFourCC tag_class;
    if(edit_options.new_tag) {
        try {
            tag_class = Invader::File::split_tag_class_extension(file_path.string()).value().fourcc;
            tag_struct = Invader::Parser::ParserStruct::generate_base_struct(tag_class);
        }
        catch (std::exception &) {
            eprintf_error("Failed to create a new tag %s. Make sure the extension is correct.", file_path.string().c_str());
            return EXIT_FAILURE;
        }
    }
    else {
        auto value = Invader::File::open_file(file_path);
        if(!value.has_value()) {
            eprintf_error("Failed to read %s", file_path.string().c_str());
            return EXIT_FAILURE;
        }
        
        try {
            tag_struct = Invader::Parser::ParserStruct::parse_hek_tag_file(value->data(), value->size());
        }
        catch (std::exception &e) {
            eprintf_error("Failed to parse %s: %s", file_path.string().c_str(), e.what());
            return EXIT_FAILURE;
        }
        
        tag_class = reinterpret_cast<const Invader::HEK::TagFileHeader *>(value->data())->tag_fourcc;
    }
    
    std::vector<std::string> output;
    bool should_save = edit_options.new_tag; // by default only save if making a new tag. this will be set to true if --set, --insert, --copy, --move, or --delete are used too
    
    for(auto &i : edit_options.actions) {
        switch(i.type) {
            case ActionType::ACTION_TYPE_LIST: {
                list_everything(populate_struct(*Invader::Parser::ParserStruct::generate_base_struct(tag_class)), output);
                break;
            }
            case ActionType::ACTION_TYPE_GET: {
                std::string bitfield;
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), bitfield, false);
                for(auto &k : arr) {
                    output.emplace_back(get_value(k, bitfield));
                }
                break;
            }
            case ActionType::ACTION_TYPE_SET: {
                std::string bitfield;
                should_save = true;
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), bitfield, edit_options.check_read_only);
                for(auto &k : arr) {
                    set_value(k, i.value, bitfield);
                }
                break;
            }
            case ActionType::ACTION_TYPE_COUNT: {
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), false);
                for(auto &k : arr) {
                    if(k.get_type() == Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
                        output.emplace_back(std::to_string(k.get_array_size()));
                    }
                    else {
                        eprintf_error("%s is not an array", k.get_member_name());
                        std::exit(EXIT_FAILURE);
                    }
                }
                break;
            }
            case ActionType::ACTION_TYPE_INSERT: {
                should_save = true;
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), edit_options.check_read_only);
                for(auto &k : arr) {
                    if(k.get_type() != Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
                        eprintf_error("%s is not an array", k.get_member_name());
                        std::exit(EXIT_FAILURE);
                    }
                    try {
                        if(i.count + k.get_array_size() > k.get_array_maximum_size()) {
                            eprintf_error("%s's maximum size of %zu exceeded", k.get_member_name(), k.get_array_maximum_size());
                            std::exit(EXIT_FAILURE);
                        }
                        k.insert_objects_in_array(i.position == SIZE_MAX ? k.get_array_size() : i.position, i.count);
                    }
                    catch(std::exception &) {
                        std::exit(EXIT_FAILURE);
                    }
                }
                break;
            }
            case ActionType::ACTION_TYPE_DELETE: {
                should_save = true;
                std::pair<std::size_t, std::size_t> range;
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), range, edit_options.check_read_only);
                for(auto &k : arr) {
                    if(k.get_type() != Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
                        eprintf_error("%s is not an array", k.get_member_name());
                        std::exit(EXIT_FAILURE);
                    }
                    try {
                        std::size_t iterations = range.second - range.first + 1;
                        if(k.get_array_size() - iterations < k.get_array_minimum_size()) {
                            eprintf_error("%s's minimum size of %zu exceeded", k.get_member_name(), k.get_array_maximum_size());
                            std::exit(EXIT_FAILURE);
                        }
                        k.delete_objects_in_array(range.first, iterations);
                    }
                    catch(std::exception &) {
                        std::exit(EXIT_FAILURE);
                    }
                }
                break;
            }
            case ActionType::ACTION_TYPE_MOVE: {
                should_save = true;
                std::pair<std::size_t, std::size_t> range;
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), range, edit_options.check_read_only);
                for(auto &k : arr) {
                    if(k.get_type() != Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
                        eprintf_error("%s is not an array", k.get_member_name());
                        std::exit(EXIT_FAILURE);
                    }
                    try {
                        std::size_t to = i.position == SIZE_MAX ? k.get_array_size() : i.position;
                        std::size_t iterations = range.second - range.first + 1;
                        if(to == range.first) {
                            continue; // we can ignore if it's trying to swap itself
                        }
                        k.swap_objects_in_array(range.first, to, iterations);
                    }
                    catch(std::exception &) {
                        std::exit(EXIT_FAILURE);
                    }
                }
                break;
            }
            case ActionType::ACTION_TYPE_COPY: {
                should_save = true;
                std::pair<std::size_t, std::size_t> range;
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), range, edit_options.check_read_only);
                for(auto &k : arr) {
                    if(k.get_type() != Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE) {
                        eprintf_error("%s is not an array", k.get_member_name());
                        std::exit(EXIT_FAILURE);
                    }
                    try {
                        std::size_t to = i.position == SIZE_MAX ? k.get_array_size() : i.position;
                        std::size_t iterations = range.second - range.first + 1;
                        
                        if(iterations + k.get_array_size() > k.get_array_maximum_size()) {
                            eprintf_error("%s's maximum size of %zu exceeded", k.get_member_name(), k.get_array_maximum_size());
                            std::exit(EXIT_FAILURE);
                        }
                        
                        k.duplicate_objects_in_array(range.first, to, iterations);
                    }
                    catch(std::exception &) {
                        std::exit(EXIT_FAILURE);
                    }
                }
                break;
            }
            default:
                eprintf_error("Unimplemented");
                std::exit(EXIT_FAILURE);
        }
    }
    
    for(auto &i : output) {
        std::printf("%s\n", i.c_str());
    }
    
    // If we're overwriting a file that isn't the main one, let's find out what
    bool create_directories_if_possible = false;
    
    if(edit_options.overwrite_path.has_value()) {
        should_save = true;
        
        auto &p = *edit_options.overwrite_path;
        auto *path = std::get_if<std::filesystem::path>(&p);
        auto *str = std::get_if<std::string>(&p);
        
        if(path) {
            file_path = *path;
        }
        else {
            file_path = edit_options.tags / *str;
            create_directories_if_possible = true; // if using a virtual path, we can create directories as needed
        }
    }
    
    if(should_save) {
        bool can_save = true;
        try {
            can_save = Invader::File::split_tag_class_extension(file_path.string()).value().fourcc == tag_class;
        }
        catch(std::exception &) {
            can_save = false;
        }
        
        if(!can_save) {
            eprintf_error("Cannot save: %s does not have the correct .%s extension", file_path.string().c_str(), Invader::HEK::tag_fourcc_to_extension(tag_class));
            return EXIT_FAILURE;
        }
        
        if(create_directories_if_possible) {
            std::error_code ec;
            std::filesystem::create_directories(file_path.parent_path(), ec);
        }
        
        if(!Invader::File::save_file(file_path, tag_struct->generate_hek_tag_data(tag_class))) {
            eprintf_error("Unable to write to %s", file_path.string().c_str());
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_SUCCESS;
    }
}
