// SPDX-License-Identifier: GPL-3.0-only

#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/header.hpp>
#include <string>

enum ActionType {
    ACTION_TYPE_GET,
    ACTION_TYPE_SET,
    ACTION_TYPE_COUNT,
    ACTION_TYPE_LIST
};

struct Actions {
    ActionType type;
    std::string key;
    std::string value;
};

static std::string get_top_member_name(const std::string &key, std::string &after_member) {
    auto *key_str = key.c_str();
    auto *c = key_str;
    for(; *c != 0 && *c != '.' && *c != '[' && *c != ']'; c++);
    
    if(c == key_str) {
        eprintf_error("Invalid key %s", key.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    auto return_value = std::string(key_str, c);;
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
    
    std::size_t min, max;
    std::size_t hyphens = 0;
    
    // Make sure it's valid
    for(auto &c : range_str) {
        if(c == '-') {
            hyphens++;
        }
        else if(c < '0' || c > '9') {
            eprintf_error("Invalid range in key %s", key.c_str());
            std::exit(EXIT_FAILURE);
        }
    }
    
    // Okay
    if(hyphens > 1) {
        eprintf_error("Invalid range in key %s", key.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    if(hyphens == 0) {
        min = std::strtoul(range_str.c_str(), nullptr, 10);
        max = min;
    }
    else {
        char *v;
        min = std::strtoul(range_str.c_str(), &v, 10);
        
        // Make sure the hyphen went somewhere
        v++;
        if(*v == 0) {
            eprintf_error("Invalid range in key %s", key.c_str());
            std::exit(EXIT_FAILURE);
        }
        
        max = std::strtoul(v, nullptr, 10);
    }
    
    // Did we exceed things?
    if(min > max) {
        eprintf_error("Invalid range in key %s", key.c_str());
        std::exit(EXIT_FAILURE);
    }
    
    return { min, max };
}

static void build_array(Invader::Parser::ParserStruct *ps, std::string key, std::vector<Invader::Parser::ParserStructValue> &array, std::string &bitfield) {
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
                auto range = get_range(key, key);
                
                if(range.first == 0 && range.second == SIZE_MAX) {
                    if(count == 0) {
                        return; // nothing to see here
                    }
                    
                    range.second = count - 1;
                }
                
                if(count < range.first || count <= range.second) {
                    eprintf_error("%zu-%zu is out of bounds for %s::%s (%zu element%s)", range.first, range.second, ps->struct_name(), member.c_str(), count, count == 1 ? "" : "s");
                    std::exit(EXIT_FAILURE);
                }
                
                for(std::size_t k = range.first; k <= range.second; k++) {
                    build_array(&i.get_object_in_array(k), key, array, bitfield);
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
                    bitfield = key.substr(1);
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

static std::vector<Invader::Parser::ParserStructValue> get_values_for_key(Invader::Parser::ParserStruct *ps, std::string key, std::string &bitfield) {
    std::vector<Invader::Parser::ParserStructValue> values;
    build_array(ps, key, values, bitfield);
    return values;
}

static std::vector<std::string> get_value_names_for_key(Invader::Parser::ParserStruct *ps, std::string key) {
    // Array!
    if(key == "") {
        std::vector<std::string> names;
        for(auto &i : ps->get_values()) {
            auto *name = i.get_member_name();
            if(name) {
                names.emplace_back(name);
            }
        }
        return names;
    }
    
    if(key[0] == '.') {
        key = std::string(key.begin() + 1, key.end());
    }
    else {
        eprintf_error("Expected a dot before key %s", key.c_str());
        std::exit(EXIT_FAILURE);
    }

    // Get the member name and range
    auto member = get_top_member_name(key, key);
    
    // Find it!
    auto values = ps->get_values();
    for(auto &i : values) {
        auto *member_name = i.get_member_name();
        
        if(member_name && member_name == member) {
            switch(i.get_type()) {
                case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE: {
                    auto range = get_range(key, key);
                
                    if(range.second != range.first) {
                        eprintf_error("Unable to enumerate multiple arrays");
                        std::exit(EXIT_FAILURE);
                    }
                    
                    auto count = i.get_array_size();
                    if(count <= range.first) {
                        eprintf_error("%zu is out of bounds for array %s", range.first, member.c_str());
                        std::exit(EXIT_FAILURE);
                    }
                    return get_value_names_for_key(&i.get_object_in_array(range.first), key);
                }
                case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM:
                case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK: {
                    if(key != "") {
                        eprintf_error("Expected end of key for enum/bitmask");
                        std::exit(EXIT_FAILURE);
                    }
                    
                    std::vector<std::string> enums;
                    for(auto &q : i.list_enum()) {
                        enums.emplace_back(q);
                    }
                    return enums;
                }
                default: {
                    eprintf_error("%s::%s is not an array, enum, or bitmask", ps->struct_name(), member.c_str());
                    std::exit(EXIT_FAILURE);
                }
            }
        }
    }
    
    eprintf_error("%s::%s does not exist", ps->struct_name(), member.c_str());
    std::exit(EXIT_FAILURE);
}

static std::string get_value(const Invader::Parser::ParserStructValue &value, const std::string &bitmask) {
    auto format = value.get_number_format();
    if(format == Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_NONE) {
        switch(value.get_type()) {
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING:
                return value.read_string();
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY:
                return Invader::File::halo_path_to_preferred_path(value.get_dependency().path) + "." + Invader::HEK::tag_class_to_extension(value.get_dependency().tag_class_int);
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_ENUM:
                return value.read_enum();
            case Invader::Parser::ParserStructValue::ValueType::VALUE_TYPE_BITMASK:
                return std::to_string(value.read_bitfield(bitmask.c_str()) ? 1 : 0);
            default:
                eprintf_error("Unsupported value type");
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
                    str += std::to_string(std::get<double>(i));
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
    
    // Something special?
    if(format == Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_NONE) {
        switch(value.get_type()) {
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
                    dep.tag_class_int = new_path.class_int;
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
                case Invader::Parser::ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT:
                    values.emplace_back(std::strtod(current_path, &c));
                    break;
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

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES
    
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the font data or tag file.");
    options.emplace_back("get", 'g', 1, "Get the value with the given key.", "<key>");
    options.emplace_back("set", 's', 2, "Set the value at the given key to the given value.", "<key> <val>");
    options.emplace_back("count", 'c', 1, "Get the number of elements in the array at the given key.", "<key>");
    options.emplace_back("list", 'l', 1, "List all the elements in the array at the given key (or the main struct if key is blank).", "<key>");

    static constexpr char DESCRIPTION[] = "Edit tags via command-line. This is intended for scripting.";
    static constexpr char USAGE[] = "[options] <tag.class>";

    struct EditOptions {
        std::filesystem::path tags = "tags";
        bool use_filesystem_path = false;
        std::vector<Actions> actions;
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
            case 'g':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_GET, arguments[0], {} });
                break;
            case 'c':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_COUNT, arguments[0], {} });
                break;
            case 'l':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_LIST, arguments[0], {} });
                break;
            case 's':
                edit_options.actions.emplace_back(Actions { ActionType::ACTION_TYPE_SET, arguments[0], arguments[1] });
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
    
    auto value = Invader::File::open_file(file_path);
    if(!value.has_value()) {
        eprintf_error("Failed to read %s", file_path.string().c_str());
        return EXIT_FAILURE;
    }
    
    std::unique_ptr<Invader::Parser::ParserStruct> tag_struct;
    
    try {
        tag_struct = Invader::Parser::ParserStruct::parse_hek_tag_file(value->data(), value->size());
    }
    catch (std::exception &e) {
        eprintf_error("Failed to parse %s: %s", file_path.string().c_str(), e.what());
        return EXIT_FAILURE;
    }
    
    std::vector<std::string> output;
    bool should_save = false;
    std::string bitfield;
    
    for(auto &i : edit_options.actions) {
        switch(i.type) {
            case ActionType::ACTION_TYPE_LIST: {
                auto arr = get_value_names_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key));
                for(auto &k : arr) {
                    output.emplace_back(k);
                }
                break;
            }
            case ActionType::ACTION_TYPE_COUNT: {
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), bitfield);
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
            case ActionType::ACTION_TYPE_GET: {
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), bitfield);
                for(auto &k : arr) {
                    output.emplace_back(get_value(k, bitfield));
                }
                break;
            }
            case ActionType::ACTION_TYPE_SET: {
                should_save = true;
                auto arr = get_values_for_key(tag_struct.get(), i.key == "" ? "" : (std::string(".") + i.key), bitfield);
                for(auto &k : arr) {
                    set_value(k, i.value, bitfield);
                }
                break;
            }
            default:
                std::exit(EXIT_FAILURE);
        }
    }
    
    for(auto &i : output) {
        std::printf("%s\n", i.c_str());
    }
    
    if(should_save) {
        return Invader::File::save_file(file_path, tag_struct->generate_hek_tag_data(reinterpret_cast<const Invader::HEK::TagFileHeader *>(value->data())->tag_class_int)) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}
