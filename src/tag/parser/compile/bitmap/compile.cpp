// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/bitmap/swizzle.hpp>

namespace Invader::Parser {
    void set_bitmap_data_environment_flag(BuildWorkload &workload, std::size_t tag_index) {
        if(workload.get_build_parameters()->details.build_cache_file_engine != HEK::CacheFileEngine::CACHE_FILE_MCC_CEA) {
            return;
        }
        
        auto &base_struct = workload.structs[*workload.tags[tag_index].base_struct];
        auto &bitmap_base = *reinterpret_cast<Parser::Bitmap::struct_little *>(base_struct.data.data());
        
        auto bitmap_data_count = static_cast<std::size_t>(bitmap_base.bitmap_data.count);
        if(bitmap_data_count == 0) {
            return;
        }
        auto *bitmap_data = reinterpret_cast<Parser::BitmapData::struct_little *>(workload.structs[*base_struct.resolve_pointer(&bitmap_base.bitmap_data.pointer)].data.data());
        
        for(std::size_t b = 0; b < bitmap_data_count; b++) {
            auto &flags = bitmap_data[b].flags;
            flags = flags.read() | HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_ENVIRONMENT;
        }
    }
    
    void BitmapData::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        auto &s = workload.structs[struct_index];
        auto *data = s.data.data();
        std::size_t bitmap_data_offset = reinterpret_cast<std::byte *>(&(reinterpret_cast<BitmapData::struct_little *>(data + offset)->bitmap_tag_id)) - data;
        this->pointer = 0xFFFFFFFF;
        this->flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_MAKE_IT_ACTUALLY_WORK;

        // Add itself as a dependency. I don't know why but apparently we need to remind ourselves that we're still ourselves.
        auto &d = s.dependencies.emplace_back();
        d.tag_index = tag_index;
        d.offset = bitmap_data_offset;
        d.tag_id_only = true;
    }

    template <typename T> static void do_pre_compile(T *bitmap, BuildWorkload &workload, std::size_t tag_index) {
        // Delete null group sequences at the end
        while(bitmap->bitmap_group_sequence.size() > 0 && bitmap->bitmap_group_sequence[bitmap->bitmap_group_sequence.size() - 1].first_bitmap_index == NULL_INDEX) {
            bitmap->bitmap_group_sequence.erase(bitmap->bitmap_group_sequence.begin() + (bitmap->bitmap_group_sequence.size() - 1));
        }
        
        // Zero out these if we're sprites (this is completely *insane* but that's what tool.exe does)
        if(bitmap->type == HEK::BitmapType::BITMAP_TYPE_SPRITES) {
            for(auto &sequence : bitmap->bitmap_group_sequence) {
                sequence.first_bitmap_index = 0;
                sequence.bitmap_count = 0;
            }
        }
        
        // Loop through again, but make sure sprites are present when needed and not present when not needed
        bool has_sprites = false;
        for(auto &sequence : bitmap->bitmap_group_sequence) {
            if((has_sprites = sequence.sprites.size() > 0)) {
                break;
            }
        }
        bool errored_on_sprites = false;
        if(has_sprites && bitmap->type != HEK::BitmapType::BITMAP_TYPE_SPRITES) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Bitmap has sprites but is not a sprites bitmap type", tag_index);
            errored_on_sprites = true;
        }
        else if(!has_sprites && bitmap->type == HEK::BitmapType::BITMAP_TYPE_SPRITES && bitmap->bitmap_data.size() > 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Bitmap with bitmap data is marked as sprites, but no sprites are present", tag_index);
            errored_on_sprites = true;
        }
        if(errored_on_sprites) {
            eprintf_warn("To fix this, recompile the bitmap");
            throw InvalidTagDataException();
        }
        
        auto max_size = bitmap->processed_pixel_data.size();
        auto *pixel_data = bitmap->processed_pixel_data.data();
        std::size_t bitmap_data_count = bitmap->bitmap_data.size();
        auto engine_target = workload.get_build_parameters()->details.build_cache_file_engine;
        auto xbox = engine_target == HEK::CacheFileEngine::CACHE_FILE_XBOX;
        
        for(std::size_t b = 0; b < bitmap_data_count; b++) {
            auto &data = bitmap->bitmap_data[b];
            bool compressed = data.flags & HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_COMPRESSED;
            
            if(data.flags & HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_SWIZZLED) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu is marked as swizzled which is unsupported for compiling maps", b);
                throw InvalidTagDataException();
            }
            
            if(
                engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION ||
                engine_target == HEK::CacheFileEngine::CACHE_FILE_RETAIL ||
                engine_target == HEK::CacheFileEngine::CACHE_FILE_DEMO
            ) {
                switch(data.format) {
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8:
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8:
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8:
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Bitmap data #%zu is monochrome which is not supported by the target engine", b);
                        break;
                    case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP:
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Bitmap data #%zu uses height map compression which is not supported by the target engine", b);
                        break;
                    default:
                        break;
                }
            }

            std::size_t data_index = &data - bitmap->bitmap_data.data();
            auto format = data.format;
            auto type = data.type;
            bool should_be_compressed = (format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1) || (format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3) || (format == HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5);

            std::size_t depth = data.depth;
            std::size_t start = data.pixel_data_offset;
            std::size_t width = data.width;
            std::size_t height = data.height;

            // Warn for stuff
            bool exceeded = false;
            bool non_power_of_two = (!HEK::is_power_of_two(height) || !HEK::is_power_of_two(width) || !HEK::is_power_of_two(depth));
            
            if(
                engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION ||
                engine_target == HEK::CacheFileEngine::CACHE_FILE_RETAIL ||
                engine_target == HEK::CacheFileEngine::CACHE_FILE_DEMO ||
                engine_target == HEK::CacheFileEngine::CACHE_FILE_XBOX
            ) {
                if(bitmap->type != HEK::BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS && non_power_of_two) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Non-interface bitmap data #%zu is non-power-of-two (%zux%zux%zu)", data_index, width, height, depth);
                }

                // Check if stock limits are exceeded
                switch(type) {
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_2D_TEXTURE:
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_WHITE:
                        if(width > 2048 || height > 2048) {
                            eprintf_warn("Bitmap data #%zu exceeds 2048x2048 (%zux%zu)", data_index, width, height);
                            exceeded = true;
                        }
                        break;
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE:
                        if(width > 256 || height > 256 || depth > 256) {
                            eprintf_warn("Bitmap data #%zu exceeds 256x256x256 (%zux%zu)", data_index, width, height);
                            exceeded = true;
                        }
                        break;
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP:
                        if(width > 512 || height > 512) {
                            eprintf_warn("Bitmap data #%zu exceeds 512x512 (%zux%zu)", data_index, width, height);
                            exceeded = true;
                        }
                        break;
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_ENUM_COUNT:
                        break;
                }
                
                // On Xbox, it's an error. Otherwise, it's a warning.
                if(exceeded) {
                    if(engine_target == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Target engine runs on a system that does not support these bitmaps");
                        throw InvalidTagDataException();
                    }
                    else {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Target engine uses D3D9; some D3D9 compliant hardware may not render this bitmap");
                    }
                }
            }

            if(depth != 1 && type != HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu is not a 3D texture but has depth (%zu != 1)", data_index, depth);
                throw InvalidTagDataException();
            }

            // Make sure these are equal
            if(compressed != should_be_compressed) {
                const char *format_name = HEK::bitmap_data_format_name(format);
                data.flags ^= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_COMPRESSED; // invert the flag in case we need to do any math on it (though it's screwed up either way)
                if(compressed) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu (format: %s) is incorrectly marked as compressed", data_index, format_name);
                    throw InvalidTagDataException();
                }
                else {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu (format: %s) is not marked as compressed", data_index, format_name);
                    throw InvalidTagDataException();
                }
            }

            auto size = HEK::size_of_bitmap(data);

            // Make sure we won't explode
            std::size_t end = start + size;
            if(start > max_size || size > max_size || end > max_size) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu range (0x%08zX - 0x%08zX) exceeds the processed pixel data size (0x%08zX)", data_index, start, end, max_size);
                throw InvalidTagDataException();
            }
            
            // If we're compiling for Xbox, we have to do this in a fun way (swizzling, cubemap memes, etc.). Otherwise, it's a straight copy
            if(xbox) {
                bool needs_swizzled = !compressed && bitmap->type != HEK::BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS;
                
                // We're going to be doing naughty things to this bitmap ;-;
                if(needs_swizzled) {
                    data.flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_SWIZZLED;
                }
                
                // The Xbox version, in compressed bitmaps, for some reason, discards all mipmaps where the width and height are less than 4 (i.e. 2x2, 1x2, 2x1, and 1x1 are discarded, so 4x2 is fine).
                std::size_t copied_mipmap_count = data.mipmap_count;
                if(compressed) {
                    // If it's already sub 4x4, we can't do anything anyway -.-
                    if(width < 4 && height < 4) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu is DXT compressed and is less than 4 pixels in both width and height, which is unsupported by the target engine", data_index);
                        throw InvalidTagDataException();
                    }
                    
                    std::size_t mipmap_width = width;
                    std::size_t mipmap_height = height;
                    std::size_t minimum_dimension = 1;
                    
                    for(copied_mipmap_count = 0; copied_mipmap_count < data.mipmap_count; copied_mipmap_count++) {
                        mipmap_width = std::max(mipmap_width / 2, minimum_dimension);
                        mipmap_height = std::max(mipmap_height / 2, minimum_dimension);
                        
                        if(mipmap_width < 4 && mipmap_height < 4) {
                            break;
                        }
                    }
                }
                
                // First, add our raw data entry
                std::size_t raw_data_index = workload.raw_data.size();
                auto &raw_data = workload.raw_data.emplace_back();
                workload.tags[tag_index].asset_data.emplace_back(raw_data_index);
                auto bits_per_pixel = HEK::calculate_bits_per_pixel(data.format);
                
                // Next, write it
                auto write_bitmap = [&raw_data, &needs_swizzled, &copied_mipmap_count, &width, &depth, &height, &compressed, &pixel_data, &start, &bits_per_pixel](std::optional<std::size_t> input_cubemap_face = std::nullopt) {
                    std::size_t mipmap_width = width;
                    std::size_t mipmap_height = height;
                    std::size_t mipmap_depth = depth;
                    std::size_t bytes_per_block;
                    std::size_t minimum_dimension = 1;
                    
                    // If DXT, convert to blocks
                    if(compressed) {
                        // Resolution it's stored as
                        if(mipmap_height % 4) {
                            mipmap_height += 4 - (mipmap_height % 4);
                        }
                        if(mipmap_width % 4) {
                            mipmap_width += 4 - (mipmap_width % 4);
                        }
                    
                        mipmap_width = std::max(mipmap_width / 4, minimum_dimension);
                        mipmap_height = std::max(mipmap_height / 4, minimum_dimension);
                        bytes_per_block = bits_per_pixel * 4 * 4 / 8;
                    }
                    else {
                        bytes_per_block = bits_per_pixel / 8;
                    }
                    
                    auto *input_start = pixel_data + start;
                    auto *input = input_start;
                    
                    for(std::size_t i = 0; i <= copied_mipmap_count; i++) {
                        auto mipmap_size = mipmap_width * mipmap_height * mipmap_depth * bytes_per_block;
                        
                        // Skip to this cubemap face if needed
                        if(input_cubemap_face.has_value()) {
                            input += (*input_cubemap_face) * mipmap_size;
                        }
                        
                        // Insert it
                        if(needs_swizzled) {
                            auto swizzled = Invader::Swizzle::swizzle(input, bits_per_pixel, mipmap_width, mipmap_height, mipmap_depth, false);
                            raw_data.insert(raw_data.end(), swizzled.begin(), swizzled.end());
                        }
                        else {
                            raw_data.insert(raw_data.end(), input, input + mipmap_size);
                        }
                        
                        // Next one
                        input += (input_cubemap_face.has_value() ? (6 - *input_cubemap_face) : 1) * mipmap_width * mipmap_depth * mipmap_height * bytes_per_block;
                        
                        // Halve dimensions
                        mipmap_height = std::max(mipmap_height / 2, minimum_dimension);
                        mipmap_width = std::max(mipmap_width / 2, minimum_dimension);
                        mipmap_depth = std::max(mipmap_depth / 2, minimum_dimension);
                    }
                    
                    // Align to 128 bytes
                    raw_data.insert(raw_data.end(), REQUIRED_PADDING_N_BYTES(raw_data.size(), HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_BITMAP_SIZE_GRANULARITY), std::byte());
                };
                
                switch(type) {
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_2D_TEXTURE:
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE:
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_WHITE:
                        write_bitmap();
                        break;
                    // If we're a cubemap, we have to separate each face into its own bitmap in sequential order, swapping the second and third faces
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP:
                        write_bitmap(0);
                        write_bitmap(2);
                        write_bitmap(1);
                        write_bitmap(3);
                        write_bitmap(4);
                        write_bitmap(5);
                        break;
                    default:
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap data #%zu has an invalid data type", data_index);
                        throw InvalidTagDataException();
                };
                
                // Lastly, set the size
                data.pixel_data_size = static_cast<std::uint32_t>(raw_data.size());
            }
            else {
                // Add it all
                std::size_t raw_data_index = workload.raw_data.size();
                workload.raw_data.emplace_back(pixel_data + start, pixel_data + end);
                workload.tags[tag_index].asset_data.emplace_back(raw_data_index);
                data.pixel_data_size = static_cast<std::uint32_t>(size);
            }
        }
    }

    template <typename T> static void do_postprocess_hek_data(T *bitmap) {
        if(bitmap->compressed_color_plate_data.size() == 0) {
            bitmap->color_plate_height = 0;
            bitmap->color_plate_width = 0;
        }
        
        for(auto &i : bitmap->bitmap_data) {
            if(bitmap->type == HEK::BitmapType::BITMAP_TYPE_INTERFACE_BITMAPS) {
                i.flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_LINEAR;
                i.flags &= ~HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_POWER_OF_TWO_DIMENSIONS; // this flag is misleading
            }
            else {
                i.flags &= ~HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_LINEAR;
                i.flags |= HEK::BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_POWER_OF_TWO_DIMENSIONS;
            }
        }
    }

    void Bitmap::postprocess_hek_data() {
        do_postprocess_hek_data(this);
    }

    void Bitmap::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        do_pre_compile(this, workload, tag_index);
    }

    template <typename T> static bool fix_power_of_two_for_tag(T &tag, bool fix) {
        bool fixed = false;
        for(auto &d : tag.bitmap_data) {
            fixed = fix_power_of_two(d, fix) || fixed;
            if(fixed && !fix) {
                return true;
            }
        }
        return fixed;
    }
}
