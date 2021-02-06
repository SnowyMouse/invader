// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/command_line_option.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>

enum Format {
    STRING_LIST_FORMAT_UNICODE,
    STRING_LIST_FORMAT_HMT,
    STRING_LIST_FORMAT_LATIN1
};

template <typename T, typename G, Invader::TagClassInt C> static std::vector<std::byte> generate_string_list_tag(const std::string &input_string) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    using namespace Invader::HEK;

    // Make the file header
    G tag_data = {};

    // Start building a list of strings
    std::vector<std::string> strings;
    std::size_t string_length = input_string.size();
    const char *c = input_string.c_str();

    // Separate into lines
    std::string string;
    std::size_t line_start = 0;
    bool character_found = false;
    static const char LINE_ENDING[] = "\r\n";
    for(std::size_t i = 0; i < string_length; i++) {
        if(c[i] == '\r' || c[i] == '\n') {
            unsigned int increment;
            if(c[i] == '\r' && c[i + 1] == '\n') {
                increment = 1;
            }
            else if(c[i] == 0) {
                eprintf_error("Error: Null character is present in the file.");
                std::exit(EXIT_FAILURE);
            }
            else {
                increment = 0;
                character_found = true;
            }
            std::string line(c + line_start, c + i);
            if(line == "###END-STRING###") {
                strings.emplace_back(string, 0, string.size() - (sizeof(LINE_ENDING) - 1));
                string.clear();
                character_found = false;
            }
            else {
                string += line + LINE_ENDING;
            }
            i += increment;
            line_start = i + 1;
        }
    }
    
    // Did we have any text left?
    if(character_found) {
        eprintf_error("Error: Missing ###END-STRING### after string.");
        std::exit(EXIT_FAILURE);
    }

    // Add each string
    for(auto &str : strings) {
        auto &new_string = tag_data.strings.emplace_back();
        auto &new_string_data = new_string.string;

        if(sizeof(T) == sizeof(char16_t)) {
            auto string_data = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(reinterpret_cast<const char *>(str.c_str()));
            new_string_data = std::vector<std::byte>(reinterpret_cast<const std::byte *>(string_data.c_str()), reinterpret_cast<const std::byte *>(string_data.c_str() + string_data.size() + 1));
        }
        else {
            new_string_data = std::vector<std::byte>(reinterpret_cast<const std::byte *>(str.c_str()), reinterpret_cast<const std::byte *>(str.c_str() + str.size() + 1));
        }
    }

    return tag_data.generate_hek_tag_data(C, true);
}

static std::vector<std::byte> generate_hud_message_text_tag(const std::string &) {
    eprintf_error("Error: Unimplemented.");
    std::exit(EXIT_FAILURE);
}

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("format", 'f', 1, "Set string list format. Can be unicode or latin-1. Must be specified if a string tag is not present.");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the text file.");

    static constexpr char DESCRIPTION[] = "Generate string list tags.";
    static constexpr char USAGE[] = "[options] <tag>";

    struct StringOptions {
        std::filesystem::path data = "data";
        std::optional<std::filesystem::path> tags;
        std::optional<Format> format;
        bool use_filesystem_path = false;
    } string_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<StringOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, string_options, [](char opt, const std::vector<const char *> &arguments, auto &string_options) {
        switch(opt) {
            case 't':
                if(string_options.tags.has_value()) {
                    eprintf_error("This tool does not support multiple tags directories.");
                    std::exit(EXIT_FAILURE);
                }
                string_options.tags = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'd':
                string_options.data = arguments[0];
                break;
            case 'P':
                string_options.use_filesystem_path = true;
                break;
            case 'f':
                if(std::strcmp(arguments[0], "utf-16") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_UNICODE;
                }
                else if(std::strcmp(arguments[0], "latin-1") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_LATIN1;
                }
                else if(std::strcmp(arguments[0], "hmt") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_HMT;
                }
                break;
        }
    });
    if(!string_options.tags.has_value()) {
        string_options.tags = "tags";
    }

    const char *valid_extension = string_options.format == Format::STRING_LIST_FORMAT_HMT ? ".hmt" : ".txt";

    // Check if there's a string tag
    std::string string_tag;
    if(string_options.use_filesystem_path) {
        std::vector<std::filesystem::path> data(&string_options.data, &string_options.data + 1);
        auto string_tag_maybe = Invader::File::file_path_to_tag_path(remaining_arguments[0], data);
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

    // Ensure it's lowercase
    for(const char *c = string_tag.c_str(); *c; c++) {
        if(*c >= 'A' && *c <= 'Z') {
            eprintf_error("Invalid tag path %s. Tag paths must be lowercase.", string_tag.c_str());
            return EXIT_FAILURE;
        }
    }

    // Make sure we have a tags directory
    std::filesystem::path tags_path(*string_options.tags);
    if(!std::filesystem::is_directory(tags_path)) {
        eprintf_error("Directory %s was not found or is not a directory", string_options.tags->string().c_str());
        return EXIT_FAILURE;
    }
    
    // If we don't have it, find it
    auto tag_path_absolute = (tags_path / string_tag);
    if(!string_options.format.has_value()) {
        int found = 0;
        if(std::filesystem::exists(tag_path_absolute.string() + std::string(".string_list"))) {
            string_options.format = Format::STRING_LIST_FORMAT_LATIN1;
            found++;
        }
        if(std::filesystem::exists(tag_path_absolute.string() + std::string(".unicode_string_list"))) {
            string_options.format = Format::STRING_LIST_FORMAT_UNICODE;
            found++;
        }
        if(found == 0) {
            eprintf_error("Cannot auto-determine format if no string list tags by the path exist");
            return EXIT_FAILURE;
        }
        else if(found > 1) {
            eprintf_error("Cannot auto-determine format if multiple string list tags by the path exist");
            return EXIT_FAILURE;
        }
    }
    
    // Make the data path
    std::filesystem::path data_path(string_options.data);
    
    // Figure out our extension
    const char *output_extension = nullptr;
    switch(*string_options.format) {
        case Format::STRING_LIST_FORMAT_HMT:
            output_extension = ".hud_message_text";
            break;
        case Format::STRING_LIST_FORMAT_LATIN1:
            output_extension = ".string_list";
            break;
        case Format::STRING_LIST_FORMAT_UNICODE:
            output_extension = ".unicode_string_list";
            break;
    }

    auto input_path = (data_path / string_tag).string() + valid_extension;
    auto output_path = tag_path_absolute.string() + output_extension;

    // Open a file
    std::FILE *f = std::fopen(input_path.c_str(), "rb");
    if(!f) {
        eprintf_error("Failed to open %s for reading.", input_path.c_str());
        return EXIT_FAILURE;
    }

    std::fseek(f, 0, SEEK_END);
    std::size_t size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    auto file_data = std::make_unique<std::byte []>(size);
    if(!std::fread(file_data.get(), size, 1, f)) {
        eprintf_error("Failed to read %s.", input_path.c_str());
        std::fclose(f);
        return EXIT_FAILURE;
    }
    std::fclose(f);

    // Determine the encoding of it
    bool source_is_utf16 = false;
    char *c = reinterpret_cast<char *>(file_data.get());
    char *c_end = c + size;
    for(char *i = c; i < c_end; i++) {
        if(*i == 0) {
            source_is_utf16 = true;
            break;
        }
    }

    // If it is utf-16, try to convert it to utf-8
    std::string text;
    if(source_is_utf16) {
        auto *data = reinterpret_cast<char16_t *>(file_data.get());
        if(*data == 0xFEFF) {
            data ++;
        }
        text = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(reinterpret_cast<char16_t *>(data));
    }
    else {
        text = std::string(reinterpret_cast<char *>(file_data.get()));
    }

    // Delete the input file data
    file_data.reset();

    // Generate the data
    std::vector<std::byte> final_data;
    switch(*string_options.format) {
        case STRING_LIST_FORMAT_UNICODE:
            final_data = generate_string_list_tag<char16_t, Invader::Parser::UnicodeStringList, Invader::TagClassInt::TAG_CLASS_UNICODE_STRING_LIST>(text);
            break;
        case STRING_LIST_FORMAT_LATIN1:
            final_data = generate_string_list_tag<char, Invader::Parser::StringList, Invader::TagClassInt::TAG_CLASS_STRING_LIST>(text);
            break;
        case STRING_LIST_FORMAT_HMT:
            final_data = generate_hud_message_text_tag(text);
            break;
    }
    
    if(final_data.size() == 0) {
        eprintf_error("String list was empty. Make sure your text file ends with a newline.");
        return EXIT_FAILURE;
    }

    // Write it all
    std::filesystem::path tag_path(output_path);

    // Create missing directories if needed
    std::error_code ec;
    std::filesystem::create_directories(tag_path.parent_path(), ec);
    
    // Save
    if(!Invader::File::save_file(output_path.c_str(), final_data)) {
        eprintf_error("Error: Failed to write to %s.", output_path.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
