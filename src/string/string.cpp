/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <getopt.h>
#include <vector>
#include <string>
#include <filesystem>
#include "../eprintf.hpp"
#include "../version.hpp"
#include "../tag/hek/header.hpp"
#include "../tag/hek/class/string_list.hpp"

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
    static struct option options[] = {
        {"help",  no_argument, 0, 'h'},
        {"info", no_argument, 0, 'i' },
        {"tags", required_argument, 0, 't' },
        {"data", required_argument, 0, 'd' },
        {"format", required_argument, 0, 'f' },
        {0, 0, 0, 0 }
    };

    int opt;
    int longindex = 0;

    const char *data = "data";
    const char *tags = "tags";
    Format format = Format::STRING_LIST_FORMAT_UTF_16;
    const char *output_extension = ".unicode_string_list";

    // Go through every argument
    while((opt = getopt_long(argc, argv, "t:d:f:hi", options, &longindex)) != -1) {
        switch(opt) {
            case 't':
                tags = optarg;
                break;
            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;
            case 'd':
                data = optarg;
                break;
            case 'f':
                if(std::strcmp(optarg, "utf-16") == 0) {
                    format = Format::STRING_LIST_FORMAT_UTF_16;
                    output_extension = ".unicode_string_list";
                }
                else if(std::strcmp(optarg, "latin-1") == 0) {
                    format = Format::STRING_LIST_FORMAT_LATIN1;
                    output_extension = ".string_list";
                }
                else if(std::strcmp(optarg, "hmt") == 0) {
                    format = Format::STRING_LIST_FORMAT_HMT;
                    output_extension = ".hud_message_text";
                }
                break;
            default:
                eprintf("Usage: %s [options] <tag>\n\n", argv[0]);
                eprintf("Generate string list tags.\n\n");
                eprintf("Options:\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --format,-f <format>         Set string list format. Can be utf-16, hmt, or\n");
                eprintf("                               or latin-1. Default: utf-16\n");
                eprintf("  --data,-d <dir>              Use the specified data directory.\n");
                eprintf("  --tags,-t <dir>              Use the specified tags directory.\n");
                return EXIT_FAILURE;
        }
    }

    // Check if there's a string tag
    const char *string_tag;
    if(optind == argc) {
        eprintf("%s: A string tag path is required. Use -h for help.\n", argv[0]);
        return EXIT_FAILURE;
    }
    else if(optind < argc - 1) {
        eprintf("%s: Unexpected argument %s\n", argv[0], argv[optind + 1]);
        return EXIT_FAILURE;
    }
    else {
        string_tag = argv[optind];
    }

    std::filesystem::path tags_path(tags);
    std::filesystem::path data_path(data);

    auto input_path = (data_path / string_tag).string() + (format == Format::STRING_LIST_FORMAT_HMT ? ".hmt" : ".txt");
    auto output_path = (tags_path / string_tag).string() + output_extension;

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
    switch(format) {
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
