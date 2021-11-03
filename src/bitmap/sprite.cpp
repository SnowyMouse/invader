// SPDX-License-Identifier: GPL-3.0-only

#include <cassert>

#include <invader/bitmap/bitmap_processor.hpp>
#include <invader/hek/data_type.hpp>

namespace Invader {
    struct SpriteSheet {
        // padding (only meaningful if >1 sprite in this sheet)
        unsigned int spacing;
        
        // max length
        unsigned int max_length;
        
        // max height? (note that non-square sprite sheets will break particles)
        std::optional<decltype(max_length)> max_height;
        
        // Bitmap data
        const GeneratedBitmapData *bitmap_data;
        
        // The sprite sheet is locked (no more sprites can be added)
        bool locked = false;
        
        struct Sprite {
            const GeneratedBitmapDataBitmap *bitmap_data;
            const SpriteSheet *sheet;
            std::size_t sprite;
            std::size_t sequence;
            unsigned int x;
            unsigned int y;
            
            Sprite(const GeneratedBitmapDataBitmap &bitmap_data, const SpriteSheet &sheet, std::size_t sprite, std::size_t sequence, unsigned int x = 0, unsigned int y = 0) noexcept :
                bitmap_data(&bitmap_data),
                sheet(&sheet),
                sprite(sprite),
                sequence(sequence),
                x(x),
                y(y) {}
            Sprite(const Sprite &) = default;
            Sprite &operator =(const Sprite &a) = default;
            
            bool overlaps(const Sprite &other) const noexcept {
                auto overlap = [](const Sprite &a, const Sprite &b) -> bool {
                    // Get edges
                    auto a_end_x = a.x + a.effective_width();
                    auto a_end_y = a.y + a.effective_height();
                    auto b_end_x = b.x + b.effective_width();
                    auto b_end_y = b.y + b.effective_height();
                    
                    // Left and top
                    auto l_border_inside = a.x >= b.x && a.x < b_end_x;
                    auto t_border_inside = a.y >= b.y && a.y < b_end_y;
                    
                    // Right and bottom
                    auto r_border_inside = a_end_x > b.x && a_end_x < b_end_x; // the "end" is actually the pixel after the last pixel and not technically inside
                    auto b_border_inside = a_end_y > b.y && a_end_y < b_end_y;
                    
                    // If both a horizontal and vertical border are inside, then it's overlapping
                    return (l_border_inside || r_border_inside) && (t_border_inside || b_border_inside);
                };
                
                return overlap(*this, other) || overlap(other, *this);
            }
            
            unsigned int effective_width() const noexcept {
                return bitmap_data->width + sheet->spacing * 2;
            }
            unsigned int effective_height() const noexcept {
                return bitmap_data->height + sheet->spacing * 2;
            }
            
            bool place_in_sheet() {
                auto max_length = this->sheet->max_length;
                
                auto width = this->effective_width();
                auto height = this->effective_height();
                
                // If the sprite is too big, fail
                if(width > max_length || height > max_length) {
                    return false;
                }
                
                auto y_end = max_length - height;
                auto x_end = max_length - width;
                
                // Otherwise, begin placing
                auto old_x = this->x;
                auto old_y = this->y;
                for(this->y = 0; this->y <= y_end; this->y++) {
                    for(this->x = 0; this->x <= x_end; this->x++) {
                        bool overlap_fail = false;
                        for(auto &s : this->sheet->sprites) {
                            if(s.overlaps(*this)) {
                                overlap_fail = true;
                                break;
                            }
                        }
                            
                        if(!overlap_fail) {
                            return true;
                        }
                    }
                }
                
                this->x = old_x;
                this->y = old_y;
                
                return false;
            }
        };
        
        std::vector<Sprite> sprites;
        
        std::vector<Pixel> bake_sprite_sheet(HEK::BitmapSpriteUsage sprite_usage) const {
            Pixel background_color;
            
            switch(sprite_usage) {
                case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_BLEND_ADD_SUBTRACT_MAX:
                    background_color.alpha = 0;
                    background_color.red = 0;
                    background_color.green = 0;
                    background_color.blue = 0;
                    break;
                case BitmapSpriteUsage::BITMAP_SPRITE_USAGE_DOUBLE_MULTIPLY:
                    background_color.alpha = 127;
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
            
            std::vector<Pixel> image(this->max_length * this->max_height.value_or(this->max_length), background_color);
            
            // If we're doing multiply, do alpha blend. Otherwise do a simple replace.
            auto blend_function = sprite_usage == BitmapSpriteUsage::BITMAP_SPRITE_USAGE_MULTIPLY_MIN ? &Pixel::alpha_blend : &Pixel::replace;
            
            // Go through each sprite
            for(auto &sprite : this->sprites) {
                auto &pixel_data = sprite.bitmap_data->pixels;
                auto width = sprite.bitmap_data->width;
                auto height = sprite.bitmap_data->height;
                
                for(std::size_t y = 0; y < height; y++) {
                    for(std::size_t x = 0; x < width; x++) {
                        auto pixel_x = x + sprite.x + this->spacing;
                        auto pixel_y = y + sprite.y + this->spacing;
                        
                        assert(pixel_y < this->max_length && pixel_x < this->max_length);
                        
                        auto &pixel_to_blend = image[pixel_x + pixel_y * this->max_length];
                        pixel_to_blend = (pixel_to_blend.*blend_function)(pixel_data[x + y * width]);
                    }
                }
            }
            
            return image;
        }
        
        // Calculate the best place to add a sprite to a sheet and return the location (if possible)
        std::optional<Sprite> best_place_to_add_sprite(std::size_t sprite, std::size_t sequence) const noexcept {
            // Instantiate this
            auto bitmap_index = bitmap_data->sequences[sequence].sprites[sprite].bitmap_index;
            Sprite sprite_candidate(bitmap_data->bitmaps[bitmap_index], *this, sprite, sequence);
            
            // Attempt to place it in the sheet
            if(sprite_candidate.place_in_sheet()) {
                return sprite_candidate;
            }
            else {
                return std::nullopt;
            }
        }
        
        bool add_sprite_to_sheet(std::size_t sprite, std::size_t sequence) {
            // If locked, don't add anymore sprites
            if(this->locked) {
                return false;
            }
            
            auto s = this->best_place_to_add_sprite(sprite, sequence);
            if(s.has_value()) {
                this->sprites.emplace_back(*s);
                return true;
            }
            return false;
        }
        
        bool add_sequence_to_sheet(const std::vector<std::size_t> &sprite_indices, std::size_t sequence) {
            auto sprite_data_backup = this->sprites;
            for(auto sprite : sprite_indices) {
                if(!this->add_sprite_to_sheet(sprite, sequence)) {
                    // Hold on. Are we adding only 1 sprite and we have no sprites currently AND that sprite would fit if we disabled the padding?
                    if(sprite_indices.size() == 1 && this->sprites.size() == 0) {
                        auto old_spacing = this->spacing;
                        this->spacing = 0;
                        
                        // If so, add it but note that no more sprites are accepted now
                        if(this->add_sprite_to_sheet(sprite, sequence)) {
                            this->locked = true;
                            return true;
                        }
                        
                        this->spacing = old_spacing;
                    }
                    
                    this->sprites = sprite_data_backup;
                    return false;
                }
            }
            return true;
        }
        
        // optimize sprite sheet size to as low as possible
        void optimize(bool allow_non_square_sprite_sheets) {
            // If we only have 1 sprite, remove spacing
            // This is very VERY horrible, inconsistent, and arbitrary as fuck (and not trivial to account for when making your sprites) but it's required to match Bungie output
            if(this->sprites.size() == 1) {
                this->spacing = 0;
            }
            
            // Get the max length needed for our current set of sprites
            decltype(this->max_length) max_length_needed = 0;
            for(auto &s : this->sprites) {
                max_length_needed = std::max(s.x + s.effective_width(), max_length_needed);
                max_length_needed = std::max(s.y + s.effective_height(), max_length_needed);
            }
            
            // Find the closest power of two (rounded up)
            this->max_length = 1;
            while(this->max_length < max_length_needed) {
                this->max_length <<= 1;
            }
            
            // If we have more than 1 sprite, brute force a smaller sprite sheet
            if(this->sprites.size() > 1 && !this->locked) {
                while(this->max_length > 1) {
                    // Copy the old values
                    auto old_max_length = this->max_length;
                    auto old_sprites = this->sprites;
                    
                    // Halve max length, clear sprites
                    this->max_length >>= 1;
                    this->sprites.clear();
                    
                    // Go through each sprite and see if we can re-add all of them again
                    for(auto s : old_sprites) {
                        // Fail - copy back in old values
                        if(!s.place_in_sheet()) {
                            this->max_length = old_max_length;
                            this->sprites = old_sprites;
                            goto done_brute_forcing_sprites;
                        }
                        
                        // Success - added!
                        this->sprites.emplace_back(s);
                    }
                }
            }
            
            // Bad
            done_brute_forcing_sprites:
            if(allow_non_square_sprite_sheets) {
                decltype(this->max_length) new_max_height = 0;
                for(auto s : sprites) {
                    new_max_height = std::max(new_max_height, s.y + s.effective_height());
                }
                
                this->max_height = 1;
                while(*this->max_height < new_max_height) {
                    *this->max_height <<= 1;
                }
            }
        }
        
        SpriteSheet(unsigned int spacing, const GeneratedBitmapData &bitmap_data, unsigned max_length) : spacing(spacing), max_length(max_length), bitmap_data(&bitmap_data) {}
        
        SpriteSheet(const SpriteSheet &other) {
            *this = other;
        }
        
        SpriteSheet &operator =(const SpriteSheet &other) {
            this->spacing = other.spacing;
            this->max_length = other.max_length;
            this->max_height = other.max_height;
            this->bitmap_data = other.bitmap_data;
            this->locked = other.locked;
            for(auto &s : other.sprites) {
                this->sprites.emplace_back(s).sheet = this;
            }
            return *this;
        }
    };
    
    std::vector<SpriteSheet> generate_sheets(std::size_t max_length, std::size_t max_sheet_count, unsigned int spacing, GeneratedBitmapData &bitmap) {
        // Reserve it
        std::vector<SpriteSheet> sprite_sheets;
        sprite_sheets.reserve(max_sheet_count); // reserve the max sheet count (performance)
        
        // Sort sprites by height in descending order
        auto sequence_count = bitmap.sequences.size();
        std::vector<std::vector<std::size_t>> sorted_sprites(sequence_count);
        for(std::size_t si = 0; si < sequence_count; si++) {
            auto &seq = bitmap.sequences[si];
            auto &sorted = sorted_sprites[si];
            auto sprite_count = seq.sprites.size();
            sorted.reserve(sprite_count);
            
            for(std::size_t s = 0; s < sprite_count; s++) {
                bool added = false;
                auto &sprite = seq.sprites[s];
                auto height = bitmap.bitmaps[sprite.original_bitmap_index].height;
                
                for(auto &s_other : sorted) {
                    auto index_other = &s_other - sorted.data();
                    
                    auto height_other = bitmap.bitmaps[seq.sprites[index_other].original_bitmap_index].height;
                    if(height_other < height) {
                        added = true;
                        sorted.insert(sorted.begin() + index_other, s);
                        break;
                    }
                }
                if(!added) {
                    sorted.emplace_back(s);
                }
            }
        }
        
        // Place them now
        std::size_t split_across = 0;
        for(std::size_t si = 0; si < sequence_count; si++) {
            // Make a new sprite sheet if we have to
            SpriteSheet new_sprite_sheet(spacing, bitmap, max_length);
            
            // Make a backup if we can't
            std::vector<SpriteSheet> backup;
            
            // Get our indices
            auto &sorted = sorted_sprites[si];
            
            // Try placing in existing sprite sheets
            for(auto &s : sprite_sheets) {
                if(s.add_sequence_to_sheet(sorted, si)) {
                    goto sequence_successfully_placed;
                }
            }
            
            // Try placing in the new sprite sheet
            if(new_sprite_sheet.add_sequence_to_sheet(sorted, si)) {
                sprite_sheets.emplace_back(new_sprite_sheet);
                goto sequence_successfully_placed;
            }
            
            // If we can't put it in a new sprite sheet, try splitting across the sprite sheets
            backup = sprite_sheets;
            split_across++;
            
            // Add it one at a time?
            for(auto sprite : sorted) {
                bool sprite_placed = false;
                auto sprite_vec = std::vector<std::size_t> { sprite };
                
                for(auto &ss : sprite_sheets) {
                    if(ss.add_sequence_to_sheet(sprite_vec, si)) {
                        sprite_placed = true;
                        break;
                    }
                }
                
                // Try adding it in a new sprite
                if(!sprite_placed && sprite_sheets.emplace_back(spacing, bitmap, max_length).add_sequence_to_sheet(sprite_vec, si)) {
                    continue;
                }
                
                // Guess not
                if(!sprite_placed) {
                    eprintf_error("Could not place all sprites in %zu %zux%zu sprite sheet%s", max_sheet_count, max_length, max_length, max_sheet_count == 1 ? "" : "s");
                    throw InvalidTagDataException();
                }
            }
            
            // Did we do it?
            sequence_successfully_placed: continue;
        }
        
        // If we split it across multiple sheets, complain but continue
        if(split_across) {
            eprintf_warn("%zu sequence%s had to be split across multiple sheets\nThis is valid but may cause issues", split_across, split_across == 1 ? "" : "s");
        }
        
        // Done
        return sprite_sheets;
    }
    
    void BitmapProcessor::process_sprites(GeneratedBitmapData &generated_bitmap, BitmapProcessorSpriteParameters &parameters, std::int16_t &mipmap_count) {
        // Get our parameters
        unsigned int spacing;
        
        // Get spacing
        if(parameters.sprite_spacing == 0) {
            spacing = mipmap_count == 1 ? 1 : 4;
        }
        else {
            spacing = parameters.sprite_spacing;
        }
        
        // Determine budgeting
        unsigned int max_sheet_length;
        unsigned int max_sheet_count;
        
        // If we have our budget count set to 0, allow the maximum possible budget
        if(parameters.sprite_budget_count == 0) {
            max_sheet_length = 1024;
            max_sheet_count = 32767;
            
            // If a sprite is bigger than a sheet, change the sheet size
            for(auto &i : generated_bitmap.bitmaps) {
                if(i.height > max_sheet_length) {
                    max_sheet_length = i.height;
                }
                if(i.width > max_sheet_length) {
                    max_sheet_length = i.width;
                }
            }
            
            // If sheet length isn't power of two, round up to the next power of two then
            if(!HEK::is_power_of_two(max_sheet_length)) {
                std::size_t n = 0;
                while(max_sheet_length) {
                    max_sheet_length >>= 1;
                    n++;
                }
                max_sheet_length = 1 << n;
            }
        }
        
        // Otherwise, set it
        else {
            max_sheet_length = parameters.sprite_budget;
            max_sheet_count = parameters.sprite_budget_count;
        }
        
        for(auto &i : generated_bitmap.sequences) {
            for(std::size_t k = i.first_bitmap; k < i.first_bitmap + i.bitmap_count; k++) {
                auto &sprite = i.sprites.emplace_back();
                auto &bitmap = generated_bitmap.bitmaps[k];
                
                sprite.original_bitmap_index = k;
                sprite.bitmap_index = k;
                sprite.registration_point_x = bitmap.registration_point_x;
                sprite.registration_point_y = bitmap.registration_point_y;
            }
        }
        
        auto sheets = generate_sheets(max_sheet_length, max_sheet_count, spacing, generated_bitmap);
        unsigned long long total_pixel_usage = 0;
        unsigned long long max_pixel_usage = max_sheet_length * max_sheet_length * max_sheet_count;
        
        // Optimize sheets. Then calculate total pixel usage
        for(auto &i : sheets) {
            i.optimize(!parameters.force_square_sprite_sheets);
            total_pixel_usage += i.max_length * i.max_height.value_or(i.max_length);
        }
        
        // Failure?
        if(total_pixel_usage > max_pixel_usage) {
            eprintf_error("Maximum budget exceeded (%llu / %llu pixels)", total_pixel_usage, max_pixel_usage);
            throw InvalidTagDataException();
        }
        
        std::vector<GeneratedBitmapDataBitmap> new_bitmaps;
        
        // Add new bitmaps
        for(auto &i : sheets) {
            std::size_t new_bitmap_index = new_bitmaps.size();
            auto &new_bitmap = new_bitmaps.emplace_back();
            std::size_t length = i.max_length;
            new_bitmap.pixels = i.bake_sprite_sheet(parameters.sprite_usage);
            new_bitmap.color_plate_x = 0;
            new_bitmap.color_plate_y = 0;
            new_bitmap.depth = 1;
            new_bitmap.width = length;
            new_bitmap.height = i.max_height.value_or(length);
            
            // Store this information
            for(auto &j : i.sprites) {
                auto &sprite = generated_bitmap.sequences[j.sequence].sprites[j.sprite];
                sprite.bitmap_index = new_bitmap_index;
                sprite.left = j.x;
                sprite.right = j.x + j.effective_width();
                sprite.top = j.y;
                sprite.bottom = j.y + j.effective_height();
                sprite.registration_point_x += i.spacing;
                sprite.registration_point_y += i.spacing;
            }
        }
        
        generated_bitmap.bitmaps = new_bitmaps;
    }
}
