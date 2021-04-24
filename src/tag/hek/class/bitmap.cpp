// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/hek/definition.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/bitmap/bitmap_encode.hpp>

namespace Invader::HEK {
    const char *BITMAP_DATA_FORMAT_NAMES[] = {
        "A8 (8-bit monochrome)",
        "Y8 (8-bit monochrome)",
        "AY8 (8-bit monochrome)",
        "A8Y8 (16-bit monochrome)",
        nullptr,
        nullptr,
        "R5G6B5 (16-bit color)",
        nullptr,
        "A1R5G5B5 (16-bit color)",
        "A4R4G4B4 (16-bit color)",
        "X8R8G8B8 (32-bit color)",
        "A8R8G8B8 (32-bit color)",
        nullptr,
        nullptr,
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

    std::size_t size_of_bitmap(const Parser::BitmapData &data) noexcept {
        return BitmapEncode::bitmap_data_size(data.width, data.height, data.depth, data.mipmap_count, data.format, data.type);
    }
}
