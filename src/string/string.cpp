// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include "../printf.hpp"
#include "../version.hpp"
#include "../tag/hek/header.hpp"
#include "../tag/hek/class/string_list.hpp"
#include "../command_line_option.hpp"
#include "../file/file.hpp"

enum Format {
    STRING_LIST_FORMAT_UTF_16,
    STRING_LIST_FORMAT_HMT,
    STRING_LIST_FORMAT_LATIN1
};

template <typename T, Invader::HEK::TagClassInt C> static std::vector<std::byte> generate_string_list_tag(const std::string &input_string) {
    using namespace Invader::HEK;

    // Make the file header
    std::vector<std::byte> tag_data(sizeof(TagFileHeader));
    *reinterpret_cast<TagFileHeader *>(tag_data.data()) = TagFileHeader(static_cast<TagClassInt>(C));

    // Start building a list of strings
    std::vector<std::string> strings;
    std::size_t string_length = input_string.size();
    const char *c = input_string.data();

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
            tag_data.insert(tag_data.end(), reinterpret_cast<std::byte *>(string_data.data()), reinterpret_cast<std::byte *>(string_data.data() + string_data.size() + 1));
        }
        else {
            string_size = str.size();
            tag_data.insert(tag_data.end(), reinterpret_cast<std::byte *>(str.data()), reinterpret_cast<std::byte *>(str.data() + str.size() + 1));
        }

        auto &data_str = reinterpret_cast<StringListString<BigEndian> *>(tag_data.data() + string_list_string_offset)->string;
        data_str.size = static_cast<std::uint32_t>(string_size + sizeof(T));
        string_list_string_offset += STRING_LIST_STRING_SIZE;
    }

    return tag_data;
}

static std::vector<std::byte> generate_hud_message_text_tag(const std::string &) {
    eprintf("Error: Unimplemented.\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char * const *argv) {
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("help", 'h', 0);
    options.emplace_back("info", 'i', 0);
    options.emplace_back("tags", 't', 1);
    options.emplace_back("data", 'd', 1);
    options.emplace_back("format", 'f', 1);
    options.emplace_back("fs-path", 'P', 0);

    struct StringOptions {
        const char *path;
        const char *data = "data";
        const char *tags = "tags";
        Format format = Format::STRING_LIST_FORMAT_UTF_16;
        const char *output_extension = ".unicode_string_list";
        bool use_filesystem_path = false;
    } string_options;

    string_options.path = argv[0];

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<StringOptions &>(argc, argv, options, 'h', string_options, [](char opt, const std::vector<const char *> &arguments, StringOptions &string_options) {
        switch(opt) {
            case 't':
                string_options.tags = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_FAILURE);
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
            default:
                eprintf("Usage: %s [options] <tag>\n\n", arguments[0]);
                eprintf("Generate string list tags.\n\n");
                eprintf("Options:\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --fs-path,-P                 Use a filesystem path for the text file.\n");
                eprintf("  --format,-f <format>         Set string list format. Can be utf-16, hmt, or\n");
                eprintf("                               or latin-1. Default: utf-16\n");
                eprintf("  --data,-d <dir>              Use the specified data directory.\n");
                eprintf("  --tags,-t <dir>              Use the specified tags directory.\n");
                std::exit(EXIT_FAILURE);
        }
    });

    const char *valid_extension = string_options.format == Format::STRING_LIST_FORMAT_HMT ? ".hmt" : ".txt";

    // Check if there's a string tag
    std::string string_tag;
    if(remaining_arguments.size() == 0) {
        eprintf("A string tag path is required. Use -h for help.\n");
        return EXIT_FAILURE;
    }
    else if(remaining_arguments.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_arguments[1]);
        return EXIT_FAILURE;
    }
    else if(string_options.use_filesystem_path) {
        std::vector<std::string> data(&string_options.data, &string_options.data + 1);
        auto string_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], data, string_options.format == Format::STRING_LIST_FORMAT_HMT ? ".hmt" : ".txt");
        if(string_tag_maybe.has_value()) {
            string_tag = string_tag_maybe.value();
        }
        else {
            eprintf("Failed to find a valid %s file %s in the data directory\n", valid_extension, remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        string_tag = remaining_arguments[0];
    }

    // Ensure it's lowercase
    for(const char *c = string_tag.data(); *c; c++) {
        if(*c >= 'A' && *c <= 'Z') {
            eprintf("Invalid tag path %s. Tag paths must be lowercase.\n", string_tag.data());
            return EXIT_FAILURE;
        }
    }

    std::filesystem::path tags_path(string_options.tags);
    std::filesystem::path data_path(string_options.data);

    auto input_path = (data_path / string_tag).string() + valid_extension;
    auto output_path = (tags_path / string_tag).string() + string_options.output_extension;

    // Open a file
    std::FILE *f = std::fopen(input_path.data(), "rb");
    if(!f) {
        eprintf("Failed to open %s for reading.\n", input_path.data());
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
            final_data = generate_string_list_tag<char16_t, Invader::HEK::TagClassInt::TAG_CLASS_UNICODE_STRING_LIST>(text);
            break;
        case STRING_LIST_FORMAT_HMT:
            final_data = generate_string_list_tag<char, Invader::HEK::TagClassInt::TAG_CLASS_STRING_LIST>(text);
            break;
        case STRING_LIST_FORMAT_LATIN1:
            final_data = generate_hud_message_text_tag(text);
            break;
    }

    // Write it all
    std::filesystem::path tag_path(output_path);
    std::filesystem::create_directories(tag_path.parent_path());
    std::FILE *tag_write = std::fopen(output_path.data(), "wb");
    if(!tag_write) {
        eprintf("Error: Failed to open %s for writing.\n", output_path.data());;
        return EXIT_FAILURE;
    }

    if(std::fwrite(final_data.data(), final_data.size(), 1, tag_write) != 1) {
        eprintf("Error: Failed to write to %s.\n", output_path.data());
        std::fclose(tag_write);
        return EXIT_FAILURE;
    }

    std::fclose(tag_write);
    return EXIT_SUCCESS;
}
