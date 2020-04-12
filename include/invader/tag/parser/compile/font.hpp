// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__FONT_HPP
#define INVADER__TAG__PARSER__COMPILE__FONT_HPP

namespace Invader::Parser {
    struct Font;
    
    /**
     * Generate character tables
     * @param font font tag to generate character tables
     */
    void generate_character_tables(Font &font);
    
    /**
     * Draw text with the given font onto a bitmap (0xAARRGGBB)
     * @param pixels    pointer to a buffer of pixels (must be at least width * height pixels)
     * @param width     width of bitmap
     * @param height    height of bitmap
     * @param text      text to render
     * @param font_data font to use
     * @param color     color to use (0xAARRGGBB)
     */
    void draw_text(std::uint32_t *pixels, std::int32_t width, std::int32_t height, const char16_t *text, const Parser::Font &font_data, std::uint32_t color = 0xFFFFFFFF);
    
    /**
     * Draw text with the given font onto a bitmap (0xAARRGGBB)
     * @param pixels    pointer to a buffer of pixels (must be at least width * height pixels)
     * @param width     width of bitmap
     * @param height    height of bitmap
     * @param text      text to render
     * @param font_data font to use
     * @param color     color to use (0xAARRGGBB)
     */
    void draw_text(std::uint32_t *pixels, std::int32_t width, std::int32_t height, const char *text, const Parser::Font &font_data, std::uint32_t color = 0xFFFFFFFF);
    
    /**
     * Get the dimensions of a bitmap with the given text and font
     * @param width     width reference to write to
     * @param height    height reference to write to
     * @param text      text to use
     * @param font_data font to use
     */
    void get_dimensions(std::int32_t &width, std::int32_t &height, const char16_t *text, const Parser::Font &font_data);
    
    /**
     * Get the dimensions of a bitmap with the given text and font
     * @param width     width reference to write to
     * @param height    height reference to write to
     * @param text      text to use
     * @param font_data font to use
     */
    void get_dimensions(std::int32_t &width, std::int32_t &height, const char *text, const Parser::Font &font_data);
}

#endif
