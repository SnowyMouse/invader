/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <getopt.h>
#include <cstdio>
#include <ft2build.h>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <vector>
#include "../tag/hek/class/font.hpp"
#include "../tag/hek/header.hpp"
#include "../eprintf.hpp"
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

int main(int argc, char *argv[]) {
    // Data directory
    const char *data = "data/";

    // Tags directory
    const char *tags = "tags/";

    // Font size
    int pixel_size = 14;

    int opt = 0, longindex = 0;

    static struct option options[] = {
        {"data",  required_argument, 0, 'd' },
        {"tags",  required_argument, 0, 't' },
        {"help",  no_argument, 0, 'h' },
        {"help",  no_argument, 0, 'i' },
        {"font-size",  required_argument, 0, 's' },
        {0, 0, 0, 0 }
    };
    while((opt = getopt_long(argc, argv, "ht:d:s:", options, &longindex)) != -1) {
        switch(opt) {
            case 'd':
                data = optarg;
                break;

            case 't':
                tags = optarg;
                break;

            case 's':
                pixel_size = static_cast<int>(std::strtol(optarg, nullptr, 10));
                if(pixel_size <= 0) {
                    eprintf("Invalid font size %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            default:
                eprintf("Usage: %s [options] <font-tag>\n\n", *argv);
                eprintf("Create font tags.\n\n");
                eprintf("Options:\n");
                eprintf("    --info,-i                  Show license and credits.\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --data,-d <path>           Set the data directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory.\n\n");
                eprintf("Font options:\n");
                eprintf("    --font-size,-s <px>        Set the font size in pixels.\n\n");
                return EXIT_FAILURE;
        }
    }

    // Make sure we have the bitmap tag path
    if(optind != argc - 1) {
        eprintf("%s: Expected a font tag path. Use -h for help.\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *font_tag = argv[optind];

    #ifndef _WIN32
    for(char *c = font_tag; *c; c++) {
        if(*c == '\\') {
            *c = '/';
        }
    }
    #endif

    // Font tag path
    std::filesystem::path tags_path(tags);
    auto tag_path = tags_path / font_tag;
    auto final_tag_path = tag_path.string() + ".font";

    // TTf path
    std::filesystem::path data_path(data);
    auto ttf_path = data_path / font_tag;
    auto final_ttf_path = ttf_path.string() + ".ttf";

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
    if(FT_Set_Pixel_Sizes(face, pixel_size, pixel_size)) {
        eprintf("Failed to set pixel size %i.\n", pixel_size);
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
            std::vector<unsigned char> data(pixel_size * pixel_size);
            float radius_inner = pixel_size / 5.0F;
            float radius_inner_distance = radius_inner * radius_inner;
            float center = pixel_size / 2.0F;

            // Make an antialiased circle of fun
            for(int x = 0; x < pixel_size; x++) {
                for(int y = 0; y < pixel_size; y++) {
                    unsigned char &c = data[x * pixel_size + y];

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
            tag_character.bitmap_height = pixel_size;
            tag_character.bitmap_width = pixel_size;
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
