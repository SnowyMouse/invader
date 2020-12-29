// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/definition.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::HEK {
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
    
    std::size_t calculate_bits_per_pixel(HEK::BitmapDataFormat format) noexcept {
        switch(format) {
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8:
                return 32;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8:
                return 16;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                return 8;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                return 4;
            default:
                return 0; // unknown
        }
    }
    
    std::size_t size_of_bitmap(std::size_t width, std::size_t height, std::size_t depth, std::size_t mipmap_count, HEK::BitmapDataFormat format, HEK::BitmapDataType type) noexcept {
        std::size_t size = 0;
        std::size_t bits_per_pixel = HEK::calculate_bits_per_pixel(format);

        // Is this a meme?
        if(bits_per_pixel == 0) {
            return 0;
        }

        bool should_be_compressed = format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1 || format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3 || format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5;
        std::size_t multiplier = type == HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP ? 6 : 1;
        std::size_t block_length = should_be_compressed ? 4 : 1;

        // Do it
        for(std::size_t i = 0; i <= mipmap_count; i++) {
            size += width * height * depth * multiplier * bits_per_pixel / 8;
            
            // Divide by 2, resetting back to 1 when needed, but make sure we don't go below the block length (4x4 if DXT, else 1x1)
            width = std::max(width / 2, block_length);
            height = std::max(height / 2, block_length);
            depth = std::max(depth / 2, static_cast<std::size_t>(1));
        }

        return size;
    }

    std::size_t size_of_bitmap(const Parser::BitmapData &data) noexcept {
        return size_of_bitmap(data.width, data.height, data.depth, data.mipmap_count, data.format, data.type);
    }
}
