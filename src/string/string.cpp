// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>

enum Format {
    STRING_LIST_FORMAT_UTF_16,
    STRING_LIST_FORMAT_HMT,
    STRING_LIST_FORMAT_LATIN1
};

template <typename T, Invader::TagClassInt C> static std::vector<std::byte> generate_string_list_tag(const std::string &input_string) {
    using namespace Invader::HEK;

    // Make the file header
    std::vector<std::byte> tag_data(sizeof(TagFileHeader));
    *reinterpret_cast<TagFileHeader *>(tag_data.data()) = TagFileHeader(static_cast<TagClassInt>(C));

    // Start building a list of strings
    std::vector<std::string> strings;
    std::size_t string_length = input_string.size();
    const char *c = input_string.c_str();

    // Separate into lines
    std::string string;
    std::size_t line_start = 0;
    static const char LINE_ENDING[] = "\r\n";
    for(std::size_t i = 0; i < string_length; i++) {
        if(c[i] == '\r' || c[i] == '\n') {
            unsigned int increment;
            if(c[i] == '\r' && c[i + 1] == '\n') {
                increment = 1;
            }
            else {
                increment = 0;
            }
            std::string line(c + line_start, c + i);
            if(line == "###END-STRING###") {
                strings.emplace_back(string, 0, string.size() - (sizeof(LINE_ENDING) - 1));
                string.clear();
            }
            else {
                string += line + LINE_ENDING;
            }
            i += increment;
            line_start = i + 1;
        }
    }

    // Make the tag header
    std::size_t header_offset = tag_data.size();
    tag_data.insert(tag_data.end(), sizeof(StringList<BigEndian>), std::byte());
    reinterpret_cast<StringList<BigEndian> *>(tag_data.data() + header_offset)->strings.count = static_cast<std::uint32_t>(strings.size());

    // Append all the tag string blocks
    std::size_t string_list_string_offset = tag_data.size();
    static constexpr std::size_t STRING_LIST_STRING_SIZE = sizeof(StringListString<BigEndian>);
    tag_data.insert(tag_data.end(), strings.size() * STRING_LIST_STRING_SIZE, std::byte());

    // Add each string
    for(auto &str : strings) {
        std::size_t string_size;

        if(sizeof(T) == sizeof(char16_t)) {
            auto string_data = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(reinterpret_cast<char *>(str.data()));
            string_size = string_data.size() * sizeof(T);
            tag_data.insert(tag_data.end(), reinterpret_cast<const std::byte *>(string_data.c_str()), reinterpret_cast<const std::byte *>(string_data.c_str() + string_data.size() + 1));
        }
        else {
            string_size = str.size();
            tag_data.insert(tag_data.end(), reinterpret_cast<const std::byte *>(str.c_str()), reinterpret_cast<const std::byte *>(str.c_str() + str.size() + 1));
        }

        auto &data_str = reinterpret_cast<StringListString<BigEndian> *>(tag_data.data() + string_list_string_offset)->string;
        data_str.size = static_cast<std::uint32_t>(string_size + sizeof(T));
        string_list_string_offset += STRING_LIST_STRING_SIZE;
    }

    return tag_data;
}

static std::vector<std::byte> generate_hud_message_text_tag(const std::string &) {
    eprintf_error("Error: Unimplemented.");
    exit(EXIT_FAILURE);
}

int main(int argc, char * const *argv) {
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("format", 'f', 1, "Set string list format. Can be utf-16, hmt, or latin-1. Default: utf-16");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the text file.");

    static constexpr char DESCRIPTION[] = "Generate string list tags.";
    static constexpr char USAGE[] = "[options] <tag>";

    struct StringOptions {
        const char *data = "data";
        const char *tags = "tags";
        Format format = Format::STRING_LIST_FORMAT_UTF_16;
        const char *output_extension = ".unicode_string_list";
        bool use_filesystem_path = false;
    } string_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<StringOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, string_options, [](char opt, const std::vector<const char *> &arguments, auto &string_options) {
        switch(opt) {
            case 't':
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
                    string_options.format = Format::STRING_LIST_FORMAT_UTF_16;
                    string_options.output_extension = ".unicode_string_list";
                }
                else if(std::strcmp(arguments[0], "latin-1") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_LATIN1;
                    string_options.output_extension = ".string_list";
                }
                else if(std::strcmp(arguments[0], "hmt") == 0) {
                    string_options.format = Format::STRING_LIST_FORMAT_HMT;
                    string_options.output_extension = ".hud_message_text";
                }
                break;
        }
    });

    const char *valid_extension = string_options.format == Format::STRING_LIST_FORMAT_HMT ? ".hmt" : ".txt";

    // Check if there's a string tag
    std::string string_tag;
    if(string_options.use_filesystem_path) {
        std::vector<std::string> data(&string_options.data, &string_options.data + 1);
        auto string_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], data, string_options.format == Format::STRING_LIST_FORMAT_HMT ? ".hmt" : ".txt");
        if(string_tag_maybe.has_value()) {
            string_tag = string_tag_maybe.value();
        }
        else {
            eprintf_error("Failed to find a valid %s file %s in the data directory\n", valid_extension, remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        string_tag = remaining_arguments[0];
    }

    // Ensure it's lowercase
    for(const char *c = string_tag.c_str(); *c; c++) {
        if(*c >= 'A' && *c <= 'Z') {
            eprintf_error("Invalid tag path %s. Tag paths must be lowercase.\n", string_tag.c_str());
            return EXIT_FAILURE;
        }
    }

    std::filesystem::path tags_path(string_options.tags);
    if(!std::filesystem::is_directory(tags_path)) {
        if(std::strcmp(string_options.tags, "tags") == 0) {
            eprintf_error("No tags directory was given, and \"tags\" was not found or is not a directory.");
        }
        else {
            eprintf_error("Directory %s was not found or is not a directory\n", string_options.tags);
        }
        return EXIT_FAILURE;
    }
    std::filesystem::path data_path(string_options.data);

    auto input_path = (data_path / string_tag).string() + valid_extension;
    auto output_path = (tags_path / string_tag).string() + string_options.output_extension;

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
    std::fread(file_data.get(), size, 1, f);
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
    switch(string_options.format) {
        case STRING_LIST_FORMAT_UTF_16:
            final_data = generate_string_list_tag<char16_t, Invader::TagClassInt::TAG_CLASS_UNICODE_STRING_LIST>(text);
            break;
        case STRING_LIST_FORMAT_HMT:
            final_data = generate_string_list_tag<char, Invader::TagClassInt::TAG_CLASS_STRING_LIST>(text);
            break;
        case STRING_LIST_FORMAT_LATIN1:
            final_data = generate_hud_message_text_tag(text);
            break;
    }

    // Write it all
    std::filesystem::path tag_path(output_path);
    
    // Create missing directories if needed
    try {
        if(!std::filesystem::exists(tag_path.parent_path())) {
            std::filesystem::create_directories(tag_path.parent_path());
        }
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to create a directory: %s\n", e.what());
        return EXIT_FAILURE;
    }

    std::FILE *tag_write = std::fopen(output_path.c_str(), "wb");
    if(!tag_write) {
        eprintf_error("Error: Failed to open %s for writing.\n", output_path.c_str());;
        return EXIT_FAILURE;
    }

    if(std::fwrite(final_data.data(), final_data.size(), 1, tag_write) != 1) {
        eprintf_error("Error: Failed to write to %s.\n", output_path.c_str());
        std::fclose(tag_write);
        return EXIT_FAILURE;
    }

    std::fclose(tag_write);
    return EXIT_SUCCESS;
}
