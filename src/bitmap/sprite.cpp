// SPDX-License-Identifier: GPL-3.0-only

#include "color_plate_scanner.hpp"

namespace Invader {
    struct SpriteReference {
        std::size_t x, y, width, height;
        std::size_t sequence, sprite;
        std::size_t half_spacing;
        
        SpriteReference(std::size_t x, std::size_t y, std::size_t width, std::size_t height, std::size_t half_spacing, std::size_t sequence, std::size_t sprite) : x(x), y(y), width(width), height(height), sequence(sequence), sprite(sprite), half_spacing(half_spacing) {}
        
        SpriteReference(SpriteReference &&) = default;
        SpriteReference(const SpriteReference &) = default;
        SpriteReference &operator =(const SpriteReference &) = default;
        
        bool inside_sprite(std::size_t x, std::size_t y) {
            // Are we on the left of it?
            if(x + this->half_spacing < this->x) {
                return false;
            }
            
            // Are we on the right of it?
            if(x >= this->x + this->width + this->half_spacing) {
                return false;
            }
            
            // Are we above it?
            if(y + this->half_spacing < this->y) {
                return false;
            }
            
            // Are we below it?
            if(y >= this->y + this->half_spacing + this->height) {
                return false;
            }
            
            // We're inside it
            return true;
        }
    };

    struct SpriteSheet {
        std::vector<SpriteReference> sprites;
        
        bool sprite_fits_at_location(const SpriteReference &sprite, std::size_t &x, std::size_t &y, bool ordered_x) {
            // Go through each pixel with each sprite
            for(auto &sprite_to_check : sprites) {
                for(std::size_t y_in_sprite_to_add = 0; y_in_sprite_to_add < sprite.height; y_in_sprite_to_add++) {
                    for(std::size_t x_in_sprite_to_add = 0; x_in_sprite_to_add < sprite.width; x_in_sprite_to_add++) {
                        if(sprite_to_check.inside_sprite(x_in_sprite_to_add + x, y_in_sprite_to_add + y)) {
                            if(!ordered_x) {
                                x = sprite_to_check.x + sprite_to_check.width + sprite_to_check.half_spacing;
                            }
                            else {
                                y = sprite_to_check.y + sprite_to_check.height + sprite_to_check.half_spacing;
                            }
                            return false;
                        }
                    }
                }
            }
            
            return true;
        }
        
        bool place_sprite(const SpriteReference &sprite, bool ordered_x, std::size_t max_length) {
            if(sprite.half_spacing > max_length) {
                return false;
            }
            
            auto length_minus_spacing = max_length - sprite.half_spacing;
            
            // Go through each pixel
            for(std::size_t a = sprite.half_spacing; a < length_minus_spacing; a++) {
                for(std::size_t b = sprite.half_spacing; b < length_minus_spacing && a < length_minus_spacing; b++) {
                    std::size_t *x;
                    std::size_t *y;
                    
                    if(ordered_x) {
                        x = &a;
                        y = &b;
                    }
                    else {
                        x = &b;
                        y = &a;
                    }
                    
                    if(sprite_fits_at_location(sprite, *x, *y, ordered_x)) {
                        auto &new_sprite = sprites.emplace_back(sprite);
                        new_sprite.x = *x;
                        new_sprite.y = *y;
                        return true;
                    }
                }
            }
            
            return false;
        }
        
        std::size_t length() const {
            std::size_t max_x = 0;
            std::size_t max_y = 0;
            
            for(auto &i : this->sprites) {
                max_x = std::max(max_x, i.x + i.half_spacing + i.width);
                max_y = std::max(max_y, i.y + i.half_spacing + i.height);
            }
            
            std::size_t max_length = std::max(max_x, max_y);
            
            // No length? Okay.
            if(max_length == 0) {
                return 0;
            }
            
            // Let's calculate this
            std::size_t max_length_copy = max_length;
            std::size_t log2_rounded_down = 0;
            
            while(max_length_copy > 1) {
                max_length_copy >>= 1;
                log2_rounded_down++;
            }
            
            // If our max length isn't power of two, then actual_max_length will be less than max_length. If so, we need to double actual_max_length and then return it.
            std::size_t actual_max_length = 1 << log2_rounded_down;
            if(max_length > actual_max_length) {
                actual_max_length *= 2;
            }
            
            return actual_max_length;
        }
        std::vector<Invader::Pixel> bake_sprite_sheet(std::size_t &length, const GeneratedBitmapData &bitmap, BitmapSpriteUsage sprite_usage) {
            // Store length
            length = this->length();
        
            // Figure out the background color, too
            Pixel background_color;
            
            switch(sprite_usage) {
                case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_BLEND_ADD_SUBTRACT_MAX:
                    background_color.alpha = 0;
                    background_color.red = 0;
                    background_color.green = 0;
                    background_color.blue = 0;
                    break;
                case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_DOUBLE_MULTIPLY:
                    background_color.alpha = 255;
                    background_color.red = 127;
                    background_color.green = 127;
                    background_color.blue = 127;
                    break;
                case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_MULTIPLY_MIN:
                    background_color.alpha = 255;
                    background_color.red = 255;
                    background_color.green = 255;
                    background_color.blue = 255;
                    break;
                case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_ENUM_COUNT:
                    std::terminate();
            }
            
            // Store this
            std::vector<Invader::Pixel> baked_sheet(length * length, background_color);
            
            // If we're doing multiply, do alpha blend. Otherwise do a simple replace.
            auto blend_function = sprite_usage == BitmapSpriteUsage::BITMAP_SPRITE_USAGE_MULTIPLY_MIN ? &Pixel::alpha_blend : &Pixel::replace;
            
            // Go through each sprite
            for(auto &sprite : this->sprites) {
                auto &pixel_data = bitmap.bitmaps[bitmap.sequences[sprite.sequence].sprites[sprite.sprite].original_bitmap_index].pixels;
                
                for(std::size_t y = 0; y < sprite.height; y++) {
                    for(std::size_t x = 0; x < sprite.width; x++) {
                        auto &pixel_to_blend = baked_sheet[x + sprite.x + (sprite.y + y) * length];
                        pixel_to_blend = (pixel_to_blend.*blend_function)(pixel_data[x + y * sprite.width]);
                    }
                }
            }
            
            // Done
            return baked_sheet;
        }
    };
    
    static std::optional<std::vector<SpriteSheet>> generate_sheets_with_direction(const std::vector<SpriteReference> &sprites_to_add, bool ordered_x, bool sprites_in_sequences_must_coexist, std::size_t max_sheets, std::size_t max_length, std::size_t sequence_count) {
        std::vector<SpriteSheet> sheets;
        sheets.reserve(max_sheets); // o.o
        
        // Add by sequence
        if(sprites_in_sequences_must_coexist) {
            auto add_all_sprites_of_sequence_to_sheet = [&sprites_to_add, &ordered_x, &max_length](std::size_t sequence, auto &sheet) -> bool {
                for(auto &sprite : sprites_to_add) {
                    // If the sprite is in the sequence, let's do the thing
                    if(sprite.sequence == sequence) {
                        if(!sheet.place_sprite(sprite, ordered_x, max_length)) { // if we fail to add it, get destroyed
                            return false;
                        }
                    }
                }
                
                return true;
            };
            
            for(std::size_t seq = 0; seq < sequence_count; seq++) {
                bool added = false;
                for(auto &sheet : sheets) {
                    auto sheet_copy = sheet;
                    if((added = add_all_sprites_of_sequence_to_sheet(seq, sheet_copy))) {
                        sheet = sheet_copy;
                        break;
                    }
                }
                
                // If we didn't add it, try a new sheet
                if(!added) {
                    // First check if we CAN. If not, fail
                    if(sheets.size() == max_sheets) {
                        return std::nullopt;
                    }
                    
                    auto &sheet = sheets.emplace_back();
                    if(!add_all_sprites_of_sequence_to_sheet(seq, sheet)) {
                        return std::nullopt;
                    }
                }
            };
        }
        
        // Add by sprite
        else {
            for(auto &sprite : sprites_to_add) {
                bool added = false;
                for(auto &sheet : sheets) {
                    if((added = sheet.place_sprite(sprite, ordered_x, max_length))) {
                        break;
                    }
                }
                
                // If we didn't add it, try a new sheet
                if(!added) {
                    // First check if we CAN. If not, fail
                    if(sheets.size() == max_sheets) {
                        return std::nullopt;
                    }
                    
                    // If adding a sprite to an empty sheet fails, give up. Not sure how this would ever fail, though, since we checked beforehand, but ok
                    if(!sheets.emplace_back().place_sprite(sprite, ordered_x, max_length)) {
                        return std::nullopt;
                    }
                }
            }
        }
            
        return sheets;
    }
    
    static std::optional<std::vector<SpriteSheet>> generate_sheets(bool sprites_in_sequences_must_coexist, unsigned int max_sheet_length, unsigned int max_sheet_count, int half_spacing, const GeneratedBitmapData &bitmap) {
        std::vector<SpriteSheet> sheets_single_sprite;
        auto sequence_count = bitmap.sequences.size();
        
        // First, go through each sprite and find sprites that can't fit or can only fit if spricing is ignored
        for(std::size_t se = 0; se < sequence_count; se++) {
            auto &sequence = bitmap.sequences[se];
            auto sprite_count = sequence.sprites.size();
            
            for(std::size_t sp = 0; sp < sprite_count; sp++) {
                auto &sprite = sequence.sprites[sp];
                auto &sprite_bitmap = bitmap.bitmaps[sprite.original_bitmap_index];
                
                // If we can't fit this sprite, give up now
                if(sprite_bitmap.height > max_sheet_length || sprite_bitmap.width > max_sheet_length) {
                    return std::nullopt;
                }
                
                // If we can only fit this sprite, check if sprites in sequences must coexist. If so, check if any other sprites are in the sequence and if so, give up. Otherwise, put the sprite in its own sheet.
                if(sprite_bitmap.height + half_spacing * 2 > max_sheet_length || sprite_bitmap.width + half_spacing * 2 > max_sheet_length) {
                    if(sprites_in_sequences_must_coexist && sequence.sprites.size() > 1) {
                        return std::nullopt;
                    }
                    
                    // If we can add even just a little bit of spacing, try that
                    std::size_t x = 0;
                    std::size_t y = 0;
                    
                    if(sprite_bitmap.width < max_sheet_length) {
                        x = (max_sheet_length - sprite_bitmap.width) / 2;
                    }
                    
                    if(sprite_bitmap.height < max_sheet_length) {
                        y = (max_sheet_length - sprite_bitmap.height) / 2;
                    }
                    
                    auto &sheet = sheets_single_sprite.emplace_back();
                    sheet.sprites.emplace_back(x, y, sprite_bitmap.width, sprite_bitmap.height, 0, se, sp);
                }
            }
        }
        
        // If we went over the limit doing that, give up
        if(sheets_single_sprite.size() > max_sheet_count) {
            return std::nullopt;
        }
        
        // Remaining sheets goes here
        std::size_t variable_sized_sheets = max_sheet_count - sheets_single_sprite.size();
        
        // Order each sprite from least to greatest
        std::vector<SpriteReference> sprites_ordered_vertical;
        std::vector<SpriteReference> sprites_ordered_horizontal;
        
        for(std::size_t se = 0; se < sequence_count; se++) {
            auto &sequence = bitmap.sequences[se];
            auto sprite_count = sequence.sprites.size();
            
            for(std::size_t sp = 0; sp < sprite_count; sp++) {
                auto &sprite = sequence.sprites[sp];
                
                // Did we already add it?
                bool already_added = false;
                for(auto &i : sheets_single_sprite) {
                    for(auto &s : i.sprites) {
                        if(s.sequence == se && s.sprite == sp) {
                            already_added = true;
                            goto spaghetti_code_monster;
                        }
                    }
                }
                
                // If we did it, pass
                spaghetti_code_monster: if(already_added) {
                    continue;
                }
                
                // Let's begin
                auto &sprite_bitmap = bitmap.bitmaps[sprite.original_bitmap_index];
                SpriteReference sprite_reference(0, 0, sprite_bitmap.width, sprite_bitmap.height, half_spacing, se, sp);
                
                // Order vertically and horizontally
                std::size_t added_sprite_count = sprites_ordered_horizontal.size();
                
                bool added_horizontal = false;
                for(std::size_t q = 0; q < added_sprite_count; q++) {
                    if(sprites_ordered_horizontal[q].width < sprite_bitmap.width) {
                        added_horizontal = true;
                        sprites_ordered_horizontal.insert(sprites_ordered_horizontal.begin() + q, sprite_reference);
                        break;
                    }
                }
                if(!added_horizontal) {
                    sprites_ordered_horizontal.emplace_back(sprite_reference);
                }
                
                bool added_vertical = false;
                for(std::size_t q = 0; q < added_sprite_count; q++) {
                    if(sprites_ordered_vertical[q].height < sprite_bitmap.height) {
                        added_vertical = true;
                        sprites_ordered_vertical.insert(sprites_ordered_vertical.begin() + q, sprite_reference);
                        break;
                    }
                }
                if(!added_vertical) {
                    sprites_ordered_vertical.emplace_back(sprite_reference);
                }
            }
        }
        
        // If we have anything remaining, generate sprite sheets
        std::vector<SpriteSheet> sheets;
        if(sprites_ordered_vertical.size() > 0) {
            // If we're exactly out of sheets, bail.
            if(variable_sized_sheets == 0) {
                return std::nullopt;
            }
            
            std::size_t length_generate = max_sheet_length;
            std::optional<std::vector<SpriteSheet>> sheets_vertical, sheets_horizontal;
            
            // If we just have one sheet, brute force a good size then
            if(variable_sized_sheets == 1 && max_sheet_count == 1) {
                while(true) {
                    auto sheets_vertical_maybe = generate_sheets_with_direction(sprites_ordered_vertical, false, sprites_in_sequences_must_coexist, variable_sized_sheets, length_generate, sequence_count);
                    auto sheets_horizontal_maybe = generate_sheets_with_direction(sprites_ordered_horizontal, true, sprites_in_sequences_must_coexist, variable_sized_sheets, length_generate, sequence_count);
                    
                    if(!sheets_vertical_maybe.has_value() && !sheets_horizontal_maybe.has_value()) {
                        break;
                    }
                    
                    sheets_vertical = sheets_vertical_maybe;
                    sheets_horizontal = sheets_horizontal_maybe;
                    
                    if(sheets_horizontal.has_value()) {
                        length_generate = std::min(length_generate, (*sheets_horizontal)[0].length());
                    }
                    
                    if(sheets_vertical.has_value()) {
                        length_generate = std::min(length_generate, (*sheets_vertical)[0].length());
                    }
                    
                    length_generate /= 2;
                }
            }
            else {
                sheets_vertical = generate_sheets_with_direction(sprites_ordered_vertical, false, sprites_in_sequences_must_coexist, variable_sized_sheets, length_generate, sequence_count);
                sheets_horizontal = generate_sheets_with_direction(sprites_ordered_horizontal, true, sprites_in_sequences_must_coexist, variable_sized_sheets, length_generate, sequence_count);
            }
            
            // Find the most efficient set of sprite sheets... if we have both. Otherwise just take whichever one was successful. Or return nothing if none were.
            if(sheets_horizontal.has_value() && sheets_vertical.has_value()) {
                // Find the most efficient set of sprite sheets we just calculated
                auto calculate_pixel_count_for_sheet_array = [](auto &what) -> std::size_t {
                    std::size_t pixel_count = 0;
                    for(auto &i : *what) {
                        auto length = i.length();
                        pixel_count += length * length;
                    }
                    return pixel_count;
                };
                
                auto pixel_count_vertical = calculate_pixel_count_for_sheet_array(sheets_vertical);
                auto pixel_count_horizontal = calculate_pixel_count_for_sheet_array(sheets_horizontal);
                
                if(pixel_count_vertical < pixel_count_horizontal) {
                    sheets = std::move(*sheets_vertical);
                }
                else {
                    sheets = std::move(*sheets_horizontal);
                }
            }
            else if(!sheets_horizontal.has_value() && sheets_vertical.has_value()) {
                sheets = *sheets_vertical;
            }
            else if(sheets_horizontal.has_value() && !sheets_vertical.has_value()) {
                sheets = *sheets_horizontal;
            }
            else {
                return std::nullopt; // nope
            }
            
            // Remove spacing from sprite sheets that have one bitmap, but only if it would save space.
            // This will fuck the sprite up on stock Halo's renderer, causing issues such as the sides of the sprites being clipped or repeated. Oh well. Bungie did it, too.
            for(auto &i : sheets) {
                if(i.sprites.size() == 1) {
                    auto &sprite = i.sprites[0];
                    auto length_before = i.length();
                    auto half_spacing_before = sprite.half_spacing;
                    sprite.x -= half_spacing_before;
                    sprite.y -= half_spacing_before;
                    sprite.half_spacing = 0;
                    
                    auto length_after = i.length();
                    if(length_after >= length_before) {
                        sprite.x += half_spacing_before;
                        sprite.y += half_spacing_before;
                        sprite.half_spacing = half_spacing_before;
                    }
                }
            }
        }
        
        sheets.insert(sheets.end(), sheets_single_sprite.begin(), sheets_single_sprite.end());
        
        return sheets;
    }
    
    static std::vector<SpriteSheet> generate_sheets(int max_sheet_length, int max_sheet_count, int half_spacing, const GeneratedBitmapData &bitmap) {
        std::vector<SpriteSheet> sheets;
        
        // Try forcing sprites in sequences to be in the same bitmap
        auto attempt1 = generate_sheets(true, max_sheet_length, max_sheet_count, half_spacing, bitmap);
        if(attempt1.has_value()) {
            return attempt1.value();
        }
        
        // Allow sprites in sequences to be in different bitmaps
        auto attempt2 = generate_sheets(false, max_sheet_length, max_sheet_count, half_spacing, bitmap);
        if(attempt2.has_value()) {
            return attempt2.value();
        }
        
        // Nope
        eprintf_error("Failed to fit sprites into %i %ix%i sprite sheets", max_sheet_count, max_sheet_length, max_sheet_length);
        throw std::exception();
    }
    
    void ColorPlateScanner::process_sprites(GeneratedBitmapData &generated_bitmap, ColorPlateScannerSpriteParameters &parameters, std::int16_t &mipmap_count) {
        // Get our parameters
        unsigned int half_spacing = 1 << mipmap_count;
        parameters.sprite_spacing = half_spacing;
        
        unsigned int max_sheet_length;
        unsigned int max_sheet_count;
        
        if(parameters.sprite_budget_count == 0) {
            max_sheet_length = 16384;
            max_sheet_count = 1;
        }
        else {
            max_sheet_length = parameters.sprite_budget;
            max_sheet_count = parameters.sprite_budget_count;
        }
        
        for(auto &i : generated_bitmap.sequences) {
            for(std::size_t k = i.first_bitmap; k < i.first_bitmap + i.bitmap_count; k++) {
                auto &sprite = i.sprites.emplace_back();
                sprite.original_bitmap_index = k;
                sprite.bitmap_index = k;
                auto &bitmap = generated_bitmap.bitmaps[k];
                sprite.registration_point_x = bitmap.registration_point_x;
                sprite.registration_point_y = bitmap.registration_point_y;
            }
        }
        
        auto sheets = generate_sheets(max_sheet_length, max_sheet_count, half_spacing, generated_bitmap);
        std::vector<GeneratedBitmapDataBitmap> new_bitmaps;
        
        // Add new bitmaps
        for(auto &i : sheets) {
            std::size_t new_bitmap_index = new_bitmaps.size();
            auto &new_bitmap = new_bitmaps.emplace_back();
            std::size_t length;
            new_bitmap.pixels = i.bake_sprite_sheet(length, generated_bitmap, parameters.sprite_usage);
            new_bitmap.color_plate_x = 0;
            new_bitmap.color_plate_y = 0;
            new_bitmap.depth = 1;
            new_bitmap.height = length;
            new_bitmap.width = length;
            
            // Store this information
            for(auto &j : i.sprites) {
                auto &sprite = generated_bitmap.sequences[j.sequence].sprites[j.sprite];
                sprite.bitmap_index = new_bitmap_index;
                sprite.left = j.x;
                sprite.right = j.x + j.width;
                sprite.top = j.y;
                sprite.bottom = j.y + j.height;
            }
        }
        
        generated_bitmap.bitmaps = new_bitmaps;
    }
}
