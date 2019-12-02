// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void BitmapData::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        auto &s = workload.structs[struct_index];
        auto *data = s.data.data();
        std::size_t bitmap_data_offset = reinterpret_cast<std::byte *>(&(reinterpret_cast<BitmapData::struct_little *>(data + offset)->bitmap_tag_id)) - data;
        auto &d = s.dependencies.emplace_back();
        d.tag_index = tag_index;
        d.offset = bitmap_data_offset;
        d.tag_id_only = true;
    }

    void Bitmap::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        for(auto &sequence : this->bitmap_group_sequence) {
            for(auto &sprite : sequence.sprites) {
                if(sprite.bitmap_index >= this->bitmap_data.size()) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sprite %zu of sequence %zu has an invalid bitmap index", &sprite - sequence.sprites.data(), &sequence - this->bitmap_group_sequence.data());
                    throw InvalidTagDataException();
                }
            }
        }

        for(auto &data : this->bitmap_data) {
            std::size_t data_index = &data - this->bitmap_data.data();
            bool compressed = data.flags.compressed;
            auto format = data.format;
            bool should_be_compressed = (format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1) || (format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3) || (format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5);

            std::size_t start = data.pixels_offset;
            std::size_t size = 0;
            std::size_t width = data.width;
            std::size_t height = data.height;
            std::size_t depth = data.type == HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE ? data.depth : 1;
            std::size_t multiplier = data.type == HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP ? 6 : 1;
            std::size_t bits_per_pixel;

            switch(data.format) {
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8:
                    bits_per_pixel = 32;
                    break;
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8:
                    bits_per_pixel = 16;
                    break;
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                    bits_per_pixel = 8;
                    break;
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                    bits_per_pixel = 4;
                    break;
                default:
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data %zu has an invalid format", data_index);
                    throw InvalidTagDataException();
            }

            // Make sure these are equal
            if(compressed != should_be_compressed) {
                const char *format_name = HEK::bitmap_data_format_name(format);
                if(compressed) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Bitmap data #%zu (format: %s) is incorrectly marked as compressed", data_index, format_name);
                }
                else {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Bitmap data #%zu (format: %s) is not marked as compressed", data_index, format_name);
                }
            }

            // Do it
            for(std::size_t i = 0; i <= data.mipmap_count; i++) {
                size += width * height * depth * multiplier * bits_per_pixel;

                // Divide by 2, resetting back to 1 when needed
                width /= 2;
                height /= 2;
                depth /= 2;
                if(width == 0) {
                    width = 1;
                }
                if(height == 0) {
                    height = 1;
                }
                if(depth == 0) {
                    depth = 1;
                }

                // If we're DXT compressed, the minimum is actually 4x4
                if(should_be_compressed) {
                    if(width < 4) {
                        width = 4;
                    }
                    else if(height < 4) {
                        height = 4;
                    }
                }
            }

            // Make sure we won't explode
            auto max_size = this->processed_pixel_data.size();
            std::size_t end = start + size;
            if(start > max_size || size > max_size || end > max_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu range (0x%08zX - 0x%08zX) exceeds the processed pixel data size (0x%08zX)", data_index, start, end, max_size);
                throw InvalidTagDataException();
            }

            // Add it all
            std::size_t raw_data_index = workload.raw_data.size();
            workload.raw_data.emplace_back(this->processed_pixel_data.data() + start, this->processed_pixel_data.data() + end);
            workload.tags[tag_index].asset_data.emplace_back(raw_data_index);
        }
    }
}
