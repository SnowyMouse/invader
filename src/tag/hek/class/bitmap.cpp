// SPDX-License-Identifier: GPL-3.0-only

#include <zlib.h>
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
    
    std::optional<std::vector<HEK::LittleEndian<std::uint32_t>>> decompress_color_plate_data(const Parser::Bitmap &data) {
        // If we don't have color plate data, return
        if(data.compressed_color_plate_data.empty()) {
            return std::nullopt;
        }
        
        // Get the size of the data we're going to decompress
        const auto *color_plate_data = data.compressed_color_plate_data.data();
        auto image_size = reinterpret_cast<const HEK::BigEndian<std::uint32_t> *>(color_plate_data)->read();
        const auto *compressed_data = color_plate_data + sizeof(image_size);
        auto compressed_size = data.compressed_color_plate_data.size() - sizeof(image_size);
        auto width = data.color_plate_width;
        auto height = data.color_plate_height;

        if(image_size != width * height * sizeof(std::uint32_t)) {
            eprintf_error("Color plate size is wrong");
            throw InvalidTagDataException();
        }
        std::vector<HEK::LittleEndian<std::uint32_t>> pixels(image_size);

        z_stream inflate_stream;
        inflate_stream.zalloc = Z_NULL;
        inflate_stream.zfree = Z_NULL;
        inflate_stream.opaque = Z_NULL;
        inflate_stream.avail_out = image_size;
        inflate_stream.next_out = reinterpret_cast<Bytef *>(pixels.data());
        inflate_stream.avail_in = compressed_size;
        inflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(compressed_data));
        
        // Do it
        if(inflateInit(&inflate_stream) != Z_OK || inflate(&inflate_stream, Z_FINISH) != Z_STREAM_END || inflateEnd(&inflate_stream) != Z_OK) {
            eprintf_error("Color plate data could not be decompressed");
            throw InvalidTagDataException();
        }
        
        return pixels;
    }
}
