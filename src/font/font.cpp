// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <ft2build.h>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <invader/tag/hek/definition.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/printf.hpp>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>
#include <invader/version.hpp>
#include FT_FREETYPE_H

struct RenderedCharacter {
    std::vector<std::byte> data;
    std::int16_t left;
    std::int16_t top;
    std::int16_t x;
    std::int16_t y;
    std::int16_t width;
    std::int16_t height;
    std::int16_t hori_advance;
};

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
    // Options struct
    struct FontOptions {
        const char *data = "data/";
        const char *tags = "tags/";
        int pixel_size = 14;
        bool use_filesystem_path = false;
    } font_options;

    // Command line options
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("data", 'd', 1, "Set the data directory.", "<dir>");
    options.emplace_back("tags", 't', 1, "Set the tags directory.", "<dir>");
    options.emplace_back("font-size", 's', 1, "Set the font size in pixels.", "<px>");
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the font file.");

    static constexpr char DESCRIPTION[] = "Create font tags from OTF/TTF files.";
    static constexpr char USAGE[] = "[options] <font-tag>";

    // Do it!
    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<FontOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, font_options, [](char opt, const auto &args, auto &font_options) {
        switch(opt) {
            case 'd':
                font_options.data = args[0];
                break;

            case 't':
                font_options.tags = args[0];
                break;

            case 'P':
                font_options.use_filesystem_path = true;
                break;

            case 's':
                font_options.pixel_size = static_cast<int>(std::strtol(args[0], nullptr, 10));
                if(font_options.pixel_size <= 0) {
                    eprintf("Invalid font size %s\n", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
                break;
        }
    });

    // Do it!
    std::string font_tag;
    FontExtension found_format = static_cast<FontExtension>(0);
    if(font_options.use_filesystem_path) {
        std::vector<std::string> data(&font_options.data, &font_options.data + 1);
        for(FontExtension i = found_format; i < FontExtension::FONT_EXTENSION_COUNT; i = static_cast<FontExtension>(i + 1)) {
            auto font_tag_maybe = Invader::File::file_path_to_tag_path_with_extension(remaining_arguments[0], data, FONT_EXTENSION_STR[i]);
            if(font_tag_maybe.has_value()) {
                font_tag = font_tag_maybe.value();
                found_format = i;
                break;
            }
        }
    }
    else {
        font_tag = remaining_arguments[0];
    }

    // Ensure it's lowercase
    for(const char *c = font_tag.data(); *c; c++) {
        if(*c >= 'A' && *c <= 'Z') {
            eprintf("Invalid tag path %s. Tag paths must be lowercase.\n", font_tag.data());
            return EXIT_FAILURE;
        }
    }

    // Font tag path
    std::filesystem::path tags_path(font_options.tags);
    auto tag_path = tags_path / font_tag;
    auto final_tag_path = tag_path.string() + ".font";

    // TTf path
    std::filesystem::path data_path(font_options.data);
    auto ttf_path = data_path / font_tag;
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
        eprintf("Failed to find a valid ttf or otf %s in the data directory.\n", remaining_arguments[0]);
        return EXIT_FAILURE;
    }

    // Load the TTF
    FT_Library library;
    FT_Face face;
    if(FT_Init_FreeType(&library)) {
        eprintf("Failed to initialize freetype.\n");
        return EXIT_FAILURE;
    }
    if(FT_New_Face(library, final_ttf_path.data(), 0, &face)) {
        eprintf("Failed to open %s.\n", final_ttf_path.data());
        return EXIT_FAILURE;
    }
    if(FT_Set_Pixel_Sizes(face, font_options.pixel_size, font_options.pixel_size)) {
        eprintf("Failed to set pixel size %i.\n", font_options.pixel_size);
        return EXIT_FAILURE;
    }

    // Render the characters in a range
    std::vector<RenderedCharacter> characters;
    for(int i = 0; i < 16384; i++) {
        auto index = FT_Get_Char_Index(face, i);
        if(FT_Load_Glyph(face, index, FT_LOAD_DEFAULT)) {
            eprintf("Failed to load glyph %i\n", i);
            return EXIT_FAILURE;
        }
        if(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
            eprintf("Failed to render glyph %i\n", i);
            return EXIT_FAILURE;
        }
        RenderedCharacter c;
        c.left = face->glyph->bitmap_left;
        c.top = face->glyph->bitmap_top;
        c.x = face->glyph->advance.x >> 6;
        c.y = face->glyph->advance.y >> 6;
        c.width = face->glyph->bitmap.width;
        c.height = face->glyph->bitmap.rows;
        c.hori_advance = face->glyph->metrics.horiAdvance / 64;

        auto *buffer = reinterpret_cast<std::byte *>(face->glyph->bitmap.buffer);
        c.data.insert(c.data.begin(), buffer, buffer + c.width * c.height);
        characters.emplace_back(c);
    }

    // Done
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    // Create
    Invader::HEK::TagFileHeader header(Invader::HEK::TagClassInt::TAG_CLASS_FONT);
    Invader::HEK::Font<Invader::HEK::BigEndian> font = {};
    std::vector<Invader::HEK::FontCharacter<Invader::HEK::BigEndian>> tag_characters;
    std::vector<std::byte> pixels;

    // Set up the character stuff
    int max_descending_height = 1;
    int max_ascending_height = 1;
    for(std::size_t i = ' '; i < characters.size(); i++) {
        auto &character = characters[i];
        Invader::HEK::FontCharacter<Invader::HEK::BigEndian> tag_character = {};

        // Dot
        if(i == 127) {
            std::vector<unsigned char> data(font_options.pixel_size * font_options.pixel_size);
            float radius_inner = font_options.pixel_size / 5.0F;
            float radius_inner_distance = radius_inner * radius_inner;
            float center = font_options.pixel_size / 2.0F;

            // Make an antialiased circle of fun
            for(int x = 0; x < font_options.pixel_size; x++) {
                for(int y = 0; y < font_options.pixel_size; y++) {
                    unsigned char &c = data[x * font_options.pixel_size + y];

                    // Subpixels
                    for(char sx = -2; sx < 3; sx++) {
                        for(char sy = -2; sy < 3; sy++) {
                            float dx = center - x + sx * 0.2;
                            float dy = center - y + sy * 0.2;

                            float distance = dx * dx + dy * dy;
                            if(distance < radius_inner_distance) {
                                c += 250 / 25;
                            }
                        }
                    }
                }
            }

            // Same as the width of the X character
            int width = characters['X'].x;
            tag_character.character = static_cast<std::uint16_t>(i);
            tag_character.bitmap_height = font_options.pixel_size;
            tag_character.bitmap_width = font_options.pixel_size;
            tag_character.character_width = width;
            tag_character.pixels_offset = static_cast<std::uint32_t>(pixels.size());
            tag_character.hardware_character_index = -1;
            tag_character.bitmap_origin_x = width / 2;
            tag_character.bitmap_origin_y = center + radius_inner * 2;
            pixels.insert(pixels.end(), reinterpret_cast<std::byte *>(data.data()), reinterpret_cast<std::byte *>(data.data()) + data.size());
        }
        else if(i == 32 || character.data.size() != 0) {
            tag_character.character = static_cast<std::uint16_t>(i);
            tag_character.bitmap_height = character.height;
            tag_character.bitmap_width = character.width;
            tag_character.character_width = character.x;
            tag_character.pixels_offset = static_cast<std::uint32_t>(pixels.size());
            tag_character.hardware_character_index = -1;
            tag_character.bitmap_origin_x = -character.left;
            tag_character.bitmap_origin_y = character.top;
            pixels.insert(pixels.end(), character.data.begin(), character.data.end());

            int descending_height = tag_character.bitmap_height - character.top;
            int ascending_height = tag_character.bitmap_height - descending_height;

            if(ascending_height > max_ascending_height) {
                max_ascending_height = ascending_height;
            }
            if(descending_height > max_descending_height) {
                max_descending_height = descending_height;
            }
        }
        else {
            continue;
        }
        tag_characters.emplace_back(tag_character);
    }

    font.ascending_height = max_ascending_height;
    font.descending_height = max_descending_height;
    font.pixels.size = pixels.size();
    font.characters.count = tag_characters.size();

    // Write
    std::filesystem::create_directories(tag_path.parent_path());
    std::FILE *f = std::fopen(final_tag_path.data(), "wb");
    if(!f) {
        eprintf("Failed to open %s for writing.\n", final_tag_path.data());
        return EXIT_FAILURE;
    }

    std::fwrite(&header, sizeof(header), 1, f);
    std::fwrite(&font, sizeof(font), 1, f);
    std::fwrite(tag_characters.data(), tag_characters.size() * sizeof(tag_characters[0]), 1, f);
    std::fwrite(pixels.data(), pixels.size(), 1, f);
    std::fclose(f);
}
