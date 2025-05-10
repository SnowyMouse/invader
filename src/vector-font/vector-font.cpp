// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <invader/tag/hek/definition.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/printf.hpp>
#include "../command_line_option.hpp"
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/version.hpp>

static const char *FONT_EXTENSION_STR[] = {
    ".ttf",
    ".otf"
};

enum FontExtension {
    FONT_EXTENSION_TTF,
    FONT_EXTENSION_OTF,

    FONT_EXTENSION_COUNT
};
static_assert(FONT_EXTENSION_COUNT == sizeof(FONT_EXTENSION_STR) / sizeof(*FONT_EXTENSION_STR));

int main(int argc, char *argv[]) {
    set_up_color_term();
    
    using namespace Invader;
    
    // Options struct
    struct VectorFontOptions {
        std::filesystem::path data = "data/";
        std::filesystem::path tags = "tags";
        bool use_filesystem_path = false;
    } vector_font_options;

    // Command line options
    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_FS_PATH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_DATA),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS)
    };

    static constexpr char DESCRIPTION[] = "Create vector_font_data tags from OTF/TTF files.";
    static constexpr char USAGE[] = "[options] <output-tag>";

      auto remaining_arguments = CommandLineOption::parse_arguments<VectorFontOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, vector_font_options, [](char opt, const auto &args, auto &vector_font_options) {
        switch(opt) {
            case 'd':
                vector_font_options.data = args[0];
                break;

            case 't':
                vector_font_options.tags = args[0];
                break;

            case 'P':
                vector_font_options.use_filesystem_path = true;
                break;

            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                break;
        }
    });

    std::string tag;
    FontExtension found_format = static_cast<FontExtension>(0);
    if(vector_font_options.use_filesystem_path) {
        auto path = std::filesystem::path(remaining_arguments[0]);
        auto tag_maybe = File::file_path_to_tag_path(path, vector_font_options.tags);
        if(tag_maybe.has_value()) {
            if(std::filesystem::path(*tag_maybe).extension() == ".vector_font_data") {
                tag = *tag_maybe;
            }
            else {
                eprintf_error("This tool only works with vector_font_data tags.");
                return EXIT_FAILURE;
            }
        }
        else {
            eprintf_error("Failed to find %s in %s", remaining_arguments[0], vector_font_options.tags.string().c_str());
            return EXIT_FAILURE;
        }
        tag = std::filesystem::path(tag).replace_extension().string();
    }
    else {
        tag = remaining_arguments[0];
    }

    // Font tag path
    std::filesystem::path tags_path(vector_font_options.tags);
    if(!std::filesystem::is_directory(tags_path)) {
        eprintf_error("Directory %s was not found or is not a directory", vector_font_options.tags.string().c_str());
        return EXIT_FAILURE;
    }
    auto tag_path = tags_path / tag;
    auto final_tag_path = tag_path.string() + ".vector_font_data";

    // TTF path
    std::filesystem::path data_path(vector_font_options.data);
    auto ttf_path = data_path / tag;
    std::string final_ttf_path;

    // Check if .ttf or .otf exists
    FontExtension ext;
    for(ext = found_format; ext < FontExtension::FONT_EXTENSION_COUNT; ext = static_cast<FontExtension>(ext + 1)) {
        final_ttf_path = ttf_path.string() + FONT_EXTENSION_STR[ext];
        if(std::filesystem::exists(final_ttf_path)) {
            break;
        }
    }
    if(ext == FontExtension::FONT_EXTENSION_COUNT) {
        eprintf_error("Failed to find a valid ttf or otf %s in the data directory.", remaining_arguments[0]);
        return EXIT_FAILURE;
    }

    // Load the font
    auto font_file = File::open_file(final_ttf_path);
    if(!font_file.has_value()) {
        eprintf_error("An error occurred while attempting to read %s", final_ttf_path.c_str());
        std::exit(EXIT_FAILURE);
    }

    auto font_file_data = std::move(*font_file);

    // Create
    Parser::VectorFontData vector_font_data = {};
    vector_font_data.format = static_cast<HEK::VectorFontDataFormat>(ext);
    auto &font_data = vector_font_data.font_data;
    font_data.insert(font_data.end(), reinterpret_cast<std::byte *>(font_file_data.data()), reinterpret_cast<std::byte *>(font_file_data.data()) + font_file_data.size());

    // Write
    std::error_code ec;
    std::filesystem::create_directories(tag_path.parent_path(), ec);
    if(!File::save_file(final_tag_path.c_str(), vector_font_data.generate_hek_tag_data(TagFourCC::TAG_FOURCC_VECTOR_FONT_DATA, true))) {
        eprintf_error("Failed to save %s.", final_tag_path.c_str());
        return EXIT_FAILURE;
    }
}
