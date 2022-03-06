// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/compile/font.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/bitmap/pixel.hpp>

namespace Invader::Parser {
    #define CALCULATE_LEADING_WIDTH(font) ((static_cast<std::int32_t>((font).ascending_height) + static_cast<std::int32_t>((font).descending_height)) / 5)
    
    void generate_character_tables(Font &font) {
        static constexpr std::uint16_t CHARACTER_TABLE_COUNT = 256;
        static constexpr std::uint16_t CHARACTER_TABLE_SIZE = 256;
        
        // Resize the base table
        font.character_tables.resize(CHARACTER_TABLE_COUNT);
        
        // Get the character count
        std::size_t character_count = std::min(font.characters.size(), static_cast<std::size_t>(NULL_INDEX - 1));
        
        // Fill anything in that was missing
        for(std::size_t t = 0; t < CHARACTER_TABLE_COUNT; t++) {
            auto &table = font.character_tables[t].character_table;
            auto last_index = table.size();
            table.resize(CHARACTER_TABLE_SIZE);
            
            if(last_index < CHARACTER_TABLE_SIZE) {
                for(std::size_t i = last_index; i < CHARACTER_TABLE_SIZE; i++) {
                    std::uint16_t character_index = (i & 0xFF) | ((t << 8) & 0xFF00);
                    auto &value = table[i].character_index;
                    value = NULL_INDEX;
                    for(std::size_t c = 0; c < character_count; c++) {
                        if(font.characters[c].character == character_index) {
                            value = c;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    void Font::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        generate_character_tables(*this);
        
        // This is super dumb, but that's what it does
        this->leading_width = CALCULATE_LEADING_WIDTH(*this);
    }

    void FontCharacter::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->hardware_character_index = NULL_INDEX;
    }
    
    static std::optional<std::size_t> get_font_character(const Parser::Font &font_data, char16_t c) {
        // Look it up in the character table if we have one present
        auto character_table_count = font_data.character_tables.size();
        if(character_table_count > 0) {
            auto unsigned_version = static_cast<std::uint16_t>(c);
            auto character_table = static_cast<std::uint8_t>(unsigned_version >> 8);
            auto &table = font_data.character_tables[character_table];
            auto ctb_size = table.character_table.size();
            auto character_table_index = static_cast<std::uint8_t>(unsigned_version);
            if(ctb_size > character_table_index) {
                auto index = table.character_table[character_table_index].character_index;
                if(index == NULL_INDEX) {
                    return std::nullopt;
                }
                else {
                    return index;
                }
            }
            return std::nullopt;
        }
        
        // Otherwise, look through each character new_color_maybe
        auto character_count = font_data.characters.size();
        for(std::size_t i = 0; i < character_count; i++) {
            auto &c_other = font_data.characters[i];
            if(c_other.character == c) {
                return i;
            }
        }
        
        return std::nullopt;
    }
    
    template <typename C> static void get_dimensions_template(std::int32_t &width, std::int32_t &height, const C *text, const Parser::Font &font_data) {
        // Initialize everything
        height = 0;
        width = 0;
        std::int32_t advance = 0;
        std::int32_t line_count = 1;
        bool last_character_was_color_thing = false;
        
        // Go through each character
        for(auto *t = text; *t; t++) {
            if(*t == '^') {
                if(!last_character_was_color_thing) {
                    last_character_was_color_thing = true;
                    continue;
                }
                else {
                    last_character_was_color_thing = false;
                }
            }
            else if(last_character_was_color_thing) {
                last_character_was_color_thing = false;
                continue;
            }
            
            if(*t == '\n') {
                advance = 0;
                line_count++;
                continue;
            }
            
            auto ci = get_font_character(font_data, *t);
            if(ci.has_value()) {
                advance += font_data.characters[*ci].character_width;
                if(advance > width) {
                    width = advance;
                }
            }
        }
        width += CALCULATE_LEADING_WIDTH(font_data);
        
        // Set the height
        height = line_count * (font_data.ascending_height + font_data.descending_height);
        
        if(width < 0) {
            width = 0;
        }
        if(height < 0) {
            height = 0;
        }
    }
    
    template <typename C> static void draw_text_template(std::uint32_t *pixels, std::int32_t width, std::int32_t height, const C *text, const Parser::Font &font_data, std::uint32_t color) {
        std::size_t line = 0;
        std::int32_t line_height = font_data.ascending_height + font_data.descending_height;
        std::int32_t horizontal_advance = 0;
        const auto *font_bitmap_data = reinterpret_cast<const std::uint8_t *>(font_data.pixels.data());
        std::size_t font_bitmap_data_length = font_data.pixels.size();
        auto font_pixel = Pixel::convert_from_32_bit(color);
        auto original_color = Pixel::convert_from_32_bit(color);
        bool last_character_was_color_thing = false;
        
        // Go through each character
        for(auto *t = text; *t; t++) {
            if(*t == '^') {
                if(!last_character_was_color_thing) {
                    last_character_was_color_thing = true;
                    continue;
                }
                else {
                    last_character_was_color_thing = false;
                }
            }
            else if(last_character_was_color_thing) {
                last_character_was_color_thing = false;
                auto new_color_maybe = Pixel::convert_from_color_code(*t);
                if(new_color_maybe.has_value()) {
                    font_pixel = *new_color_maybe;
                }
                else {
                    font_pixel = original_color;
                }
                continue;
            }
            
            if(*t == '\n') {
                line++;
                horizontal_advance = 0;
                continue;
            }
            
            auto ci = get_font_character(font_data, *t);
            auto leading_width = CALCULATE_LEADING_WIDTH(font_data);
            if(ci.has_value()) {
                auto &c = font_data.characters[*ci];
                
                // Get the x offset
                std::int32_t bx = horizontal_advance - (c.bitmap_origin_x - leading_width);
                std::int32_t by = font_data.ascending_height - (c.bitmap_origin_y + font_data.leading_height) + line_height * line;
                
                horizontal_advance += c.character_width;
                auto bitmap_width = c.bitmap_width;
                std::int32_t bxw = bx + bitmap_width;
                std::int32_t byh = by + c.bitmap_height;
                
                std::size_t pixels_required = bitmap_width * c.bitmap_height;
                std::size_t pixels_start = c.pixels_offset;
                
                if(pixels_start + pixels_required > font_bitmap_data_length) {
                    continue;
                }
                
                std::int32_t start_x = bx < 0 ? 0 : bx;
                std::int32_t start_y = by < 0 ? 0 : by;
                
                for(std::int32_t y = start_y; y < byh && y < height; y++) {
                    for(std::int32_t x = start_x; x < bxw && x < width; x++) {
                        auto &resulting_pixel = pixels[x + y * width];
                        font_pixel.alpha = font_bitmap_data[x - bx + (y - by) * bitmap_width + pixels_start];
                        resulting_pixel = Pixel::convert_from_32_bit(resulting_pixel).alpha_blend(font_pixel).convert_to_32_bit();
                    }
                }
            }
        }
    }
    
    void draw_text(std::uint32_t *pixels, std::int32_t width, std::int32_t height, const char16_t *text, const Parser::Font &font_data, std::uint32_t color) {
        draw_text_template(pixels, width, height, text, font_data, color);
    }
    
    void draw_text(std::uint32_t *pixels, std::int32_t width, std::int32_t height, const char *text, const Parser::Font &font_data, std::uint32_t color) {
        draw_text_template(pixels, width, height, text, font_data, color);
    }
    
    void get_dimensions(std::int32_t &width, std::int32_t &height, const char16_t *text, const Parser::Font &font_data) {
        get_dimensions_template(width, height, text, font_data);
    }
    
    void get_dimensions(std::int32_t &width, std::int32_t &height, const char *text, const Parser::Font &font_data) {
        get_dimensions_template(width, height, text, font_data);
    }
}
