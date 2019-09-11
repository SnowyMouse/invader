/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "bitmap.hpp"

namespace Invader::HEK {
    void compile_bitmap_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Bitmap);

        ASSERT_SIZE(tag.compressed_color_plate_data.size);
        INCREMENT_DATA_PTR(tag.compressed_color_plate_data.size);

        auto *processed_data = data;
        ASSERT_SIZE(tag.processed_pixel_data.size);
        compiled.asset_data.insert(compiled.asset_data.begin(), processed_data, processed_data + tag.processed_pixel_data.size);
        INCREMENT_DATA_PTR(tag.processed_pixel_data.size);

        // Get all of the data. We'll need to comb over it in a bit.
        std::vector<BitmapGroupSequence<LittleEndian>> sequence_data;
        std::vector<BitmapGroupSprite<LittleEndian>> sprite_data;
        skip_data = true;
        ADD_REFLEXIVE_START(tag.bitmap_group_sequence) {
            sequence_data.push_back(reflexive);
            ADD_REFLEXIVE_START(reflexive.sprites) {
                sprite_data.push_back(reflexive);
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END
        skip_data = false;

        // Check the data
        std::size_t last_sequence;
        for(last_sequence = 0; last_sequence < sequence_data.size(); last_sequence++) {
            auto &sequence = sequence_data[last_sequence];
            if(sequence.first_bitmap_index == -1) {
                break;
            }
        }

        // Remove anything at and after last_sequence
        while(last_sequence < sequence_data.size()) {
            sequence_data.erase(sequence_data.end() - 1);
        }

        // Next, calculate the offsets of the sequence and sprite data that's used
        std::size_t sequence_offset = compiled.data.size();
        std::size_t sprite_offset = compiled.data.size() + sizeof(*sequence_data.data()) * last_sequence;
        std::size_t sprite_index = 0;
        for(std::size_t s = 0; s < last_sequence; s++) {
            auto &sequence = sequence_data[s];
            std::size_t sprite_count = sequence.sprites.count;
            if(sprite_count) {
                add_pointer(compiled, sequence_offset + s * sizeof(sequence) + (reinterpret_cast<std::byte *>(&sequence.sprites.pointer) - reinterpret_cast<std::byte *>(&sequence)), sprite_offset);
                sprite_index += sprite_count;
                sprite_offset += sizeof(*sprite_data.data()) * sprite_count;
            }
        }

        // Add all the data and make the pointer a reality
        compiled.data.insert(compiled.data.end(), reinterpret_cast<std::byte *>(sequence_data.data()), reinterpret_cast<std::byte *>(sequence_data.data() + sequence_data.size()));
        compiled.data.insert(compiled.data.end(), reinterpret_cast<std::byte *>(sprite_data.data()), reinterpret_cast<std::byte *>(sprite_data.data() + sprite_data.size()));
        tag.bitmap_group_sequence.count = static_cast<std::uint32_t>(last_sequence);
        add_pointer(compiled, reinterpret_cast<std::byte *>(&tag.bitmap_group_sequence.pointer) - reinterpret_cast<std::byte *>(&tag), sequence_offset);

        ADD_REFLEXIVE_START(tag.bitmap_data) {
            reflexive.pointer = -1;
            auto flags = reflexive.flags.read();
            flags.external = 0;
            flags.make_it_actually_work = 1;
            reflexive.flags.write(flags);

            std::size_t total_pixel_count = 0;
            std::size_t total_pixel_count_dxt = 0;
            std::size_t total_mipmap_count = reflexive.mipmap_count.read();

            // Calculate number of pixels
            std::size_t mipmap_width = reflexive.width;
            std::size_t mipmap_height = reflexive.height;
            for(std::size_t mipmap = 0; mipmap <= total_mipmap_count; mipmap++) {
                total_pixel_count += mipmap_height * mipmap_width;
                total_pixel_count_dxt += (mipmap_width < 4 ? 4 : mipmap_width) * (mipmap_height <= 4 ? 4 : mipmap_height);

                mipmap_height /= 2;
                mipmap_width /= 2;
            }

            // Now, set the size based on number of pixels
            switch(reflexive.format.read()) {
                // 8-bit
                case BITMAP_FORMAT_A8:
                case BITMAP_FORMAT_Y8:
                case BITMAP_FORMAT_P8_BUMP:
                case BITMAP_FORMAT_AY8:
                    reflexive.pixels_count = static_cast<std::uint32_t>(total_pixel_count);
                    break;

                // 8-bit DXT (4x4 blocks)
                case BITMAP_FORMAT_DXT3:
                case BITMAP_FORMAT_DXT5:
                    reflexive.pixels_count = static_cast<std::uint32_t>(total_pixel_count_dxt);
                    break;

                // 16-bit
                case BITMAP_FORMAT_A8Y8:
                case BITMAP_FORMAT_R5G6B5:
                case BITMAP_FORMAT_A1R5G5B5:
                case BITMAP_FORMAT_A4R4G4B4:
                    reflexive.pixels_count = static_cast<std::uint32_t>(total_pixel_count * 2);
                    break;

                // 32-bit
                case BITMAP_FORMAT_X8R8G8B8:
                case BITMAP_FORMAT_A8R8G8B8:
                    reflexive.pixels_count = static_cast<std::uint32_t>(total_pixel_count * 4);
                    break;

                // 4-bit DXT (4x4 blocks)
                case BITMAP_FORMAT_DXT1:
                    reflexive.pixels_count = static_cast<std::uint32_t>(total_pixel_count_dxt / 2);
                    break;

                // lol
                default:
                    reflexive.pixels_count = 0;
            }

            switch(reflexive.type.read()) {
                case BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP:
                    reflexive.pixels_count = reflexive.pixels_count * 6;
                    break;
                case BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE:
                    reflexive.pixels_count = reflexive.pixels_count * reflexive.depth.read();
                    break;
                default:
                    break;
            }
        } ADD_REFLEXIVE_END

        FINISH_COMPILE;
    }

    const char *BITMAP_DATA_FORMAT_NAMES[] = {
        "A8 (8-bit monochrome)",
        "Y8 (8-bit monochrome)",
        "AY8 (8-bit monochrome)",
        "A8Y8 (16-bit monochrome)",
        "Halo Custom Edition is better than Halo CE Anniversary. And that's sad because Halo Custom Edition, itself, sucks.",
        "But you know what the saddest part is? Nobody who owns the rights to Halo CE will *ever* fix it. They would have done it a long time ago.",
        "R5G6B5 (16-bit color)",
        "We've fixed numerous issues with Halo Custom Edition through modding. We don't even have access to the source code.",
        "A1R5G5B5 (16-bit color)",
        "A4R4G4B4 (16-bit color)",
        "X8R8G8B8 (32-bit color)",
        "A8R8G8B8 (32-bit color)",
        "But I suppose we'll have to accept the fact that our idea of proper Halo CE is too much from official means. The game we love is not the same as 343I's game.",
        "Long live Halo CE... or whatever approximation they're labeling Halo CE these days.",
        "DXT1 (4 bpp S3TC)",
        "DXT3 (8 bpp S3TC)",
        "DXT5 (8 bpp S3TC)",
        "P8 (8-bit palette)"
    };

    const char *bitmap_data_format_name(BitmapDataFormat format) {
        return format >= (sizeof(BITMAP_DATA_FORMAT_NAMES) / sizeof(*BITMAP_DATA_FORMAT_NAMES)) ? nullptr : BITMAP_DATA_FORMAT_NAMES[format];
    }
}
