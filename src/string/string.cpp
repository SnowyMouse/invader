// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include "../command_line_option.hpp"
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>

using namespace Invader;

enum Format {
    STRING_LIST_FORMAT_UNICODE,
    STRING_LIST_FORMAT_HMT,
    STRING_LIST_FORMAT_1252
};

template <typename OUTPUT_TYPE, typename TAG_STRUCT, TagFourCC FOURCC> static std::vector<std::byte> generate_string_list_tag(const std::u16string &input_string) {
    using namespace HEK;

    // Make the file header
    TAG_STRUCT tag_data = {};

    // Start building a list of strings
    std::vector<std::u16string> strings;
    std::size_t string_length = input_string.size();
    const auto *c = input_string.c_str();

    // Separate into lines
    std::u16string string;
    std::size_t line_start = 0;
    bool middle_of_string = false;
    static const char16_t LINE_ENDING[] = u"\r\n";
    for(std::size_t i = 0; i < string_length; i++) {
        if(c[i] == 0) {
            eprintf_error("Error: Null character is present in the file.");
            std::exit(EXIT_FAILURE);
        }
        if(c[i] == '\r' || c[i] == '\n') {
            unsigned int increment;
            if(c[i] == '\r') {
                if(c[i + 1] != '\n') {
                    eprintf_error("Error: CR with unaccompanied LF character.");
                    std::exit(EXIT_FAILURE);
                }
                
                increment = 1;
            }
            else {
                increment = 0;
            }
            
            std::u16string line(c + line_start, c + i);
            middle_of_string = true;
            if(line == u"###END-STRING###") {
                strings.emplace_back(string, 0, string.size() - 2); // -2 for CRLF
                string.clear();
                middle_of_string = false;
            }
            else {
                string += line + LINE_ENDING;
            }
            
            i += increment;
            line_start = i + 1;
        }
        else {
            middle_of_string = true;
        }
    }
    
    // Did we have any text left?
    if(middle_of_string) {
        eprintf_error("Error: Missing ###END-STRING### after string.");
        std::exit(EXIT_FAILURE);
    }

    // Add each string
    for(auto &str : strings) {
        auto &new_string = tag_data.strings.emplace_back();
        auto &new_string_data = new_string.string;
        
        auto *string_start = str.c_str();
        auto *string_end = string_start + str.size() + 1;

        if(sizeof(OUTPUT_TYPE) == sizeof(char16_t)) {
            auto *string_start_byte = reinterpret_cast<const std::byte *>(string_start);
            auto *string_end_byte = reinterpret_cast<const std::byte *>(string_end);
            new_string_data = std::vector<std::byte>(string_start_byte, string_end_byte);
        }
        else {
            std::vector<std::byte> converted_8bit_text;
            converted_8bit_text.reserve(string_end - string_start);
            for(auto *s = string_start; s < string_end; s++) {
                converted_8bit_text.emplace_back(static_cast<std::byte>(*s));
            }
            new_string_data = std::move(converted_8bit_text);
        }
    }
    
    oprintf_success("Generated a string list with %zu string%s.", strings.size(), strings.size() == 1 ? "" : "s");

    return tag_data.generate_hek_tag_data(FOURCC, true);
}

static std::vector<std::byte> generate_hud_message_text_tag(const std::u16string &str) {
    std::vector<std::pair<std::string, std::u16string>> strings;
    
    std::size_t line_start = 0;
    std::size_t i = line_start;
    auto string_length = str.size();
    const auto *c = str.c_str();
    
    while(true) {
        if(c[i] == 0 && i != string_length) {
            eprintf_error("Null character is present in the file.");
            std::exit(EXIT_FAILURE);
        }
        
        // End of line?
        if(c[i] == '\r' || c[i] == '\n' || c[i] == 0) {
            unsigned int increment;
            if(c[i] == '\r') {
                if(c[i + 1] != '\n') {
                    eprintf_error("CR with unaccompanied LF character.");
                    std::exit(EXIT_FAILURE);
                }
                increment = 1;
            }
            else {
                increment = 0;
            }
            
            bool found_equals = false;
            auto line_start_str = c + line_start;
            for(auto *j = line_start_str; j < c + i; j++) {
                if(*j == '=') {
                    found_equals = true;
                    
                    auto key16 = std::u16string(line_start_str, j);
                    std::string key;
                    key.reserve(sizeof(key16));
                    for(auto i : key16) {
                        key.push_back(i);
                    }
                    
                    auto value = std::u16string(j + 1, c + i);
                    for(auto &s : strings) {
                        if(s.first == key) {
                            eprintf_error("Error: Duplicate %s keys.", key.c_str());
                            std::exit(EXIT_FAILURE);
                        }
                    }
                    strings.emplace_back(key, value);
                    
                    break;
                }
            }
            if(!found_equals) {
                eprintf_error("Line %zu needs to be formatted as key=value be valid.", strings.size() + 1);
                std::exit(EXIT_FAILURE);
            }
                
            i += increment;
            line_start = i + 1;
        }
        
        // Break on null terminator
        if(c[i] == 0) {
            break;
        }
        
        i++;
    }
    
    // Make the thing
    Invader::Parser::HUDMessageText tag_data = {};
    tag_data.messages.reserve(strings.size());
    
    std::vector<char16_t> text_data;
    
    for(auto &s : strings) {
        auto &m = tag_data.messages.emplace_back();
        auto &k = s.first;
        auto *key_str = k.c_str();
        auto key_length = k.size();
        if(key_length >= sizeof(m.name)) {
            eprintf_error("Key '%s' exceeds %zu characters", key_str, sizeof(m.name));
            std::exit(EXIT_FAILURE);
        }
        std::strncpy(m.name.string, key_str, key_length);
        
        m.start_index_into_text_blob = text_data.size();
        
        auto start_element = tag_data.message_elements.size();
        m.start_index_of_message_block = start_element;
        
        auto *message_str = s.second.c_str();
        auto *word_start = message_str;
        for(auto *c = message_str; true; c++) {
            // Control
            if(*word_start == '%' && (*c == ' ' || *c == 0)) {
                std::u16string thing = { word_start + 1, c };
                static constexpr const char16_t *ALL_MESSAGE_TYPES[] = {
                    u"a-button",
                    u"b-button",
                    u"x-button",
                    u"y-button",
                    u"black-button",
                    u"white-button",
                    u"left-trigger",
                    u"right-trigger",
                    u"dpad-up",
                    u"dpad-down",
                    u"dpad-left",
                    u"dpad-right",
                    u"start-button",
                    u"back-button",
                    u"left-thumb",
                    u"right-thumb",
                    u"left-stick",
                    u"right-stick",
                    u"action",
                    u"throw-grenade",
                    u"primary-trigger",
                    u"integrated-light",
                    u"jump",
                    u"use-equipment",
                    u"rotate-weapons",
                    u"rotate-grenades",
                    u"crouch",
                    u"zoom",
                    u"accept",
                    u"back",
                    u"move",
                    u"look",
                    u"custom-1",
                    u"custom-2",
                    u"custom-3",
                    u"custom-4",
                    u"custom-5",
                    u"custom-6",
                    u"custom-7",
                    u"custom-8"
                };
                
                bool found = false;
                for(std::size_t i = 0; i < sizeof(ALL_MESSAGE_TYPES) - sizeof(ALL_MESSAGE_TYPES[0]); i++) {
                    if(thing == ALL_MESSAGE_TYPES[i]) {
                        auto &element = tag_data.message_elements.emplace_back();
                        element.type = 1;
                        element.data = i;
                        found = true;
                        break;
                    }
                }
                
                if(!found) {
                    std::string c(thing.begin(), thing.end());
                    eprintf_error("Unknown control code %s.", c.c_str());
                    std::exit(EXIT_FAILURE);
                }
                
                word_start = c;
            }
            
            // Non-control
            else if(*word_start != '%' && (*c == '%' || *c == 0)) {
                if(c != word_start) {
                    auto &element = tag_data.message_elements.emplace_back();
                    element.type = 0;
                    element.data = (c - word_start) + 1;
                    text_data.insert(text_data.end(), word_start, c); // insert the string
                    text_data.emplace_back(); // and the null terminator
                }
                word_start = c;
            }
            
            if(*c == 0) {
                break;
            }
        }
        
        m.panel_count = tag_data.message_elements.size() - start_element;
    }
    
    auto text_data_bytes_start = text_data.data();
    auto text_data_bytes_end = text_data_bytes_start + text_data.size();
    
    tag_data.text_data = { reinterpret_cast<std::byte *>(text_data_bytes_start), reinterpret_cast<std::byte *>(text_data_bytes_end) };
    
    oprintf_success("Generated a HUD message text list with %zu string%s.", strings.size(), strings.size() == 1 ? "" : "s");
    
    return tag_data.generate_hek_tag_data(HEK::TagFourCC::TAG_FOURCC_HUD_MESSAGE_TEXT, true);
}

int main(int argc, char * const *argv) {
    set_up_color_term();
    
    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_DATA),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS),
        CommandLineOption("group", 'G', 1, "Specify the type of string tag. Can be: hud_message_text, string_list, unicode_string_list", "<group>"),
    };

    static constexpr char DESCRIPTION[] = "Generate string list tags.";
    static constexpr char USAGE[] = "[options] -G <group> <tag>";

    struct StringOptions {
        std::filesystem::path data = "data";
        std::filesystem::path tags = "tags";
        std::optional<Format> format;
        bool use_filesystem_path = false;
    } string_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<StringOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, string_options, [](char opt, const std::vector<const char *> &arguments, auto &string_options) {
        switch(opt) {
            case 't':
                string_options.tags = arguments[0];
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'd':
                string_options.data = arguments[0];
                break;
            case 'P':
                string_options.use_filesystem_path = true;
                break;
            case 'G':
                if(std::strcmp(arguments[0], "unicode_string_list") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_UNICODE;
                }
                else if(std::strcmp(arguments[0], "string_list") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_1252;
                }
                else if(std::strcmp(arguments[0], "hud_message_text") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_HMT;
                }
                else {
                    eprintf_error("Invalid group %s. Use -h for more information.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });
    
    if(!string_options.format.has_value()) {
        eprintf_error("No tag group specified. Use -h for more information.");
        return EXIT_FAILURE;
    }

    const char *valid_extension = string_options.format == Format::STRING_LIST_FORMAT_HMT ? ".hmt" : ".txt";

    // Check if there's a string tag
    std::string string_tag;
    if(string_options.use_filesystem_path) {
        std::vector<std::filesystem::path> data(&string_options.data, &string_options.data + 1);
        auto string_tag_maybe = File::file_path_to_tag_path(remaining_arguments[0], data);
        if(string_tag_maybe.has_value() && std::filesystem::exists(remaining_arguments[0])) {
            string_tag = std::filesystem::path(*string_tag_maybe).replace_extension().string();
        }
        else {
            eprintf_error("Failed to find a valid %s file %s in the data directory", valid_extension, remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        string_tag = remaining_arguments[0];
    }
    
    // Figure out our extension
    const char *output_extension = nullptr;
    switch(*string_options.format) {
        case Format::STRING_LIST_FORMAT_HMT:
            output_extension = ".hud_message_text";
            break;
        case Format::STRING_LIST_FORMAT_1252:
            output_extension = ".string_list";
            break;
        case Format::STRING_LIST_FORMAT_UNICODE:
            output_extension = ".unicode_string_list";
            break;
    }

    auto input_path = (string_options.data / string_tag).string() + valid_extension;
    auto output_path = string_options.tags / (string_tag + output_extension);
    
    auto file_data = Invader::File::open_file(input_path);
    if(!file_data.has_value()) {
        eprintf_error("Failed to read %s", input_path.c_str());
        return EXIT_FAILURE;
    }

    // Determine the encoding of it
    auto *input_data = file_data->data();
    auto bom = reinterpret_cast<std::uint16_t *>(input_data);
    auto input_size = file_data->size();
    std::u16string chars_16bit;
    
    bool is_16_bit = false;
    bool is_host_endian = false;
    
    auto *start16 = reinterpret_cast<char16_t *>(bom + 1);
    auto *end16 = reinterpret_cast<char16_t *>(input_data + input_size);
    
    auto *start8 = reinterpret_cast<char8_t *>(input_data);
    auto *end8 = reinterpret_cast<char8_t *>(input_data + input_size);
    
    // Is it big enough to hold a BOM? If so, read it.
    if(input_size >= sizeof(*bom)) {
        if(*bom == 0xFEFF) {
            is_host_endian = true;
            is_16_bit = true;
        }
        else if(*bom == 0xFFFE) {
            is_host_endian = false;
            is_16_bit = true;
        }
    }
    
    // If it's 16-bit, parse it as such.
    if(is_16_bit) {
        if(((reinterpret_cast<std::uintptr_t>(end16) - reinterpret_cast<std::uintptr_t>(start16)) % sizeof(*start16)) != 0) {
            eprintf_error("File %s has a 16-bit BOM but is not actually 16-bit", input_path.c_str());
            return EXIT_FAILURE;
        }
        chars_16bit = { start16, end16 };
        if(!is_host_endian) {
            for(auto &c : chars_16bit) {
                auto u16 = static_cast<std::uint16_t>(c);
                c = static_cast<char16_t>((u16 << 8) | (u16 >> 8));
            }
        }
    }
    else {
        if(*string_options.format == Format::STRING_LIST_FORMAT_HMT) {
            eprintf_error("File %s is not 16-bit, but .hmt files must be 16-bit", input_path.c_str());
            return EXIT_FAILURE;
        }
        chars_16bit.reserve(input_size);
        for(auto *i = start8; i < end8; i++) {
            chars_16bit.push_back(*i);
        }
    }

    // Generate the data
    std::vector<std::byte> final_data;
    switch(*string_options.format) {
        case STRING_LIST_FORMAT_UNICODE:
            final_data = generate_string_list_tag<char16_t, Parser::UnicodeStringList, TagFourCC::TAG_FOURCC_UNICODE_STRING_LIST>(chars_16bit);
            break;
        case STRING_LIST_FORMAT_1252:
            final_data = generate_string_list_tag<char8_t, Parser::StringList, TagFourCC::TAG_FOURCC_STRING_LIST>(chars_16bit);
            break;
        case STRING_LIST_FORMAT_HMT:
            final_data = generate_hud_message_text_tag(chars_16bit);
            break;
    }

    // Write it all
    std::filesystem::path tag_path(output_path);

    // Create missing directories if needed
    std::error_code ec;
    std::filesystem::create_directories(tag_path.parent_path(), ec);
    
    // Save
    if(!File::save_file(output_path.c_str(), final_data)) {
        eprintf_error("Error: Failed to write to %s.", output_path.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
