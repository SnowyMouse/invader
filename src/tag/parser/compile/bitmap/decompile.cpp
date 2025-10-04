// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/bitmap/swizzle.hpp>
#include <invader/bitmap/pixel.hpp>

namespace Invader::Parser {
    void Bitmap::post_cache_parse(const Invader::Tag &tag, std::optional<HEK::Pointer>) {
        this->postprocess_hek_data();

        auto &map = tag.get_map();
        auto engine = map.get_cache_version();
        auto xbox = engine == HEK::CacheFileEngine::CACHE_FILE_XBOX;
        auto &base_struct = tag.get_base_struct<HEK::Bitmap>();

        // Un-zero out these if we're sprites (again, this is completely *insane* but compiled maps have this zeroed out for whatever reason which can completely FUCK things up if this were to not be "sprites" all of a sudden)
        if(this->type == HEK::BitmapType::BITMAP_TYPE_SPRITES) {
            for(auto &sequence : this->bitmap_group_sequence) {
                // Default
                sequence.first_bitmap_index = NULL_INDEX;
                sequence.bitmap_count = sequence.sprites.size() == 1 ? 1 : 0; // set to 1 if we have one sprite; 0 otherwise

                // If we have sprites, find the lowest bitmap index of them all
                if(sequence.sprites.size() != 0) {
                    for(auto &sprite : sequence.sprites) {
                        sequence.first_bitmap_index = std::min(sequence.first_bitmap_index, sprite.bitmap_index);
                    }
                }
            }
        }

        // Do we have bitmap data?
        auto bd_count = this->bitmap_data.size();
        if(bd_count) {
            auto *bitmap_data_le_array = tag.resolve_reflexive(base_struct.bitmap_data);

            for(std::size_t bd = 0; bd < bd_count; bd++) {
                auto &bitmap_data = this->bitmap_data[bd];
                auto &bitmap_data_le = bitmap_data_le_array[bd];
                std::size_t size = bitmap_data.pixel_data_size;

                bool compressed = bitmap_data.flags & HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_COMPRESSED;
                bool should_be_compressed = false;

                switch(bitmap_data.format) {
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_BC7:
                        should_be_compressed = true;
                        break;
                    default:
                        should_be_compressed = false;
                }

                // Bad!
                if(should_be_compressed != compressed) {
                    if(compressed) {
                        eprintf_error("Bitmap is incorrectly marked as compressed but is NOT DXT; tag is corrupt");
                    }
                    else {
                        eprintf_error("Bitmap is incorrectly NOT marked as compressed but is DXT; tag is corrupt");
                    }
                    throw InvalidTagDataException();
                }

                // Also check if it needs deswizzled (don't do it yet)
                bool swizzled = bitmap_data.flags & HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_SWIZZLED;
                if(swizzled) {
                    if(compressed) {
                        eprintf_error("Bitmap is incorrectly marked as compressed AND swizzled; tag is corrupt");
                        throw InvalidTagDataException();
                    }
                }

                // Nope
                if(bitmap_data.depth != 1 && bitmap_data.type != HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE) {
                    eprintf_error("Bitmap has depth but is not a 3D texture");
                    throw InvalidTagDataException();
                }
                if(!HEK::is_power_of_two(bitmap_data.depth)) {
                    eprintf_error("Bitmap depth is non-power-of-two");
                    throw InvalidTagDataException();
                }

                // Check that compressed bitmaps are modulo block size on Xbox
                if(xbox && compressed) {
                    bool valid = bitmap_data.width % 4 == 0 && bitmap_data.height % 4 == 0;
                    if(!valid) {
                        eprintf_error("Compressed bitmap data %zu dimensions (%ux%u) are not modulo block size; tag is corrupt", bd, bitmap_data.width, bitmap_data.height);
                        throw InvalidTagDataException();
                    }
                }

                // Get it!
                const std::byte *bitmap_data_ptr;
                if(bitmap_data_le.flags.read() & HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_EXTERNAL) {
                    bitmap_data_ptr = map.get_data_at_offset(bitmap_data.pixel_data_offset, size, Map::DATA_MAP_BITMAP);
                }
                else {
                    bitmap_data_ptr = map.get_internal_asset(bitmap_data.pixel_data_offset, size);
                }

                // Xbox buffer (for Xbox bitmaps), since the size will always be a multiple of 128 and thus won't be the same size as a PC bitmap
                std::vector<std::byte> xbox_to_pc_buffer;

                if(xbox) {
                    if(compressed) {
                        // On Xbox compressed mipmaps smaller than 4px in any dimension are garbage.
                        std::uint16_t real_mipmap_count = bitmap_data.mipmap_count;
                        while(((bitmap_data.height >> real_mipmap_count) < 4 || (bitmap_data.width >> real_mipmap_count) < 4) && real_mipmap_count > 0) {
                            real_mipmap_count--;
                        }

                        bitmap_data.mipmap_count = real_mipmap_count;
                    }

                    // Set flag as unswizzled
                    bitmap_data.flags = bitmap_data.flags & ~HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_SWIZZLED;

                    // Set our buffer up
                    xbox_to_pc_buffer.resize(HEK::size_of_bitmap(bitmap_data));

                    auto *bitmap = this;

                    auto copy_texture = [&xbox_to_pc_buffer, &swizzled, &compressed, &bitmap_data_ptr, &size, &bitmap_data, &bitmap](std::optional<std::size_t> input_offset = std::nullopt, std::optional<std::size_t> output_cubemap_face = std::nullopt) -> std::size_t {
                        std::size_t bits_per_pixel = calculate_bits_per_pixel(bitmap_data.format);
                        auto *base_input = bitmap_data_ptr + input_offset.value_or(0);
                        auto *input = base_input;

                        auto *output = xbox_to_pc_buffer.data();

                        // Offset the output
                        if(output_cubemap_face.has_value()) {
                            output += (bitmap_data.height * bitmap_data.width * bitmap_data.depth * bits_per_pixel) / 8 * *output_cubemap_face;
                        }

                        std::size_t mipmap_width = bitmap_data.width;
                        std::size_t mipmap_height = bitmap_data.height;
                        std::size_t mipmap_depth = bitmap_data.depth;

                        // Copy this stuff
                        for(std::size_t m = 0; m <=  bitmap_data.mipmap_count; m++) {
                            std::size_t mipmap_size = mipmap_width * mipmap_height * mipmap_depth * bits_per_pixel / 8;

                            // Bitmap data is out of bounds?
                            if((input - base_input) + mipmap_size > size) {
                                throw OutOfBoundsException();
                            }

                            // Swizzle that stuff!
                            if(swizzled) {
                                auto swizzled_data = Invader::Swizzle::swizzle(input, bits_per_pixel, mipmap_width, mipmap_height, mipmap_depth, true);
                                std::memcpy(output, swizzled_data.data(), mipmap_size);
                            }
                            else {
                                std::memcpy(output, input, mipmap_size);
                            }

                            // Continue...
                            std::size_t stride_count = output_cubemap_face.has_value() ? (6 - *output_cubemap_face) : 1;
                            output += stride_count * mipmap_size;
                            input += mipmap_size;

                            // Halve the dimensions
                            std::size_t minimum_dimension = 1;
                            mipmap_width = std::max(mipmap_width / 2, minimum_dimension);
                            mipmap_height = std::max(mipmap_height / 2, minimum_dimension);
                            mipmap_depth = std::max(mipmap_depth / 2, minimum_dimension);

                            // Skip that data, too
                            if(output_cubemap_face.has_value()) {
                                mipmap_size = mipmap_width * mipmap_height * mipmap_depth * bits_per_pixel / 8;
                                output += *output_cubemap_face * mipmap_size;
                            }
                        }

                        return input - base_input;
                    };

                    // Cubemaps store each face as individual bitmaps rather than by mipmap, swapping the second and third faces.
                    auto copy_cube_map = [&copy_texture]() -> void {
                        std::size_t offset = 0;
                        for(std::size_t i = 0; i < 6; i++) {
                            int to_i;

                            // Swap the second and third faces
                            if(i == 1) {
                                to_i = 2;
                            }
                            else if(i == 2) {
                                to_i = 1;
                            }
                            else {
                                to_i = i;
                            }

                            offset += copy_texture(offset, to_i);

                            // Add some padding since bitmaps are stored with sizes module 128
                            offset += REQUIRED_PADDING_N_BYTES(offset, HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_BITMAP_SIZE_GRANULARITY);
                        }
                    };

                    switch(bitmap_data.type) {
                        case HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP:
                            copy_cube_map();
                            break;
                        case HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE:
                            copy_texture();
                            break;
                        case HEK::BitmapDataType::BITMAP_DATA_TYPE_WHITE:
                        case HEK::BitmapDataType::BITMAP_DATA_TYPE_2D_TEXTURE:
                            copy_texture();
                            break;
                        default:
                            throw std::exception();
                            break;
                    }

                    bitmap_data_ptr = xbox_to_pc_buffer.data();
                    size = xbox_to_pc_buffer.size();
                    bitmap_data.pixel_data_size = size;
                }

                // Calculate our offset
                bitmap_data.pixel_data_offset = static_cast<std::size_t>(this->processed_pixel_data.size());

                // Insert this
                this->processed_pixel_data.insert(this->processed_pixel_data.end(), bitmap_data_ptr, bitmap_data_ptr + size);
            }
        }
    }
}
