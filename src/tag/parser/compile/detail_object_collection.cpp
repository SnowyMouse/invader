// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void DetailObjectCollection::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        // Get the struct
        auto &this_struct = workload.structs[struct_index];
        auto &struct_data = *reinterpret_cast<struct_little *>(this_struct.data.data() + offset);
        
        // Do we need to do anything?
        std::size_t type_count = struct_data.types.count.read();
        if(type_count == 0) {
            return;
        }
        auto *types = reinterpret_cast<DetailObjectCollectionObjectType::struct_little *>(workload.structs[*this_struct.resolve_pointer(&struct_data.types.pointer)].data.data());
        
        // Just set the sprite counts to 0 if we're not actually building a map
        if(workload.disable_recursion) {
            for(std::size_t t = 0; t < type_count; t++) {
                types[t].sprite_count = 0;
            }
            return;
        }
        
        // Get sprite plate stuff
        auto &sprite_plate_tag = workload.tags[struct_data.sprite_plate.tag_id.read().index];
        auto sprite_plate_struct = workload.structs[*sprite_plate_tag.base_struct];
        auto &sprite_plate_data = *reinterpret_cast<HEK::Bitmap<HEK::LittleEndian> *>(sprite_plate_struct.data.data());
        
        // Get sequence information
        std::size_t sequence_count = sprite_plate_data.bitmap_group_sequence.count.read();
        auto sequences_struct_index = sprite_plate_struct.resolve_pointer(&sprite_plate_data.bitmap_group_sequence.pointer);
        auto *sequences = sequences_struct_index.has_value() ? reinterpret_cast<HEK::BitmapGroupSequence<HEK::LittleEndian> *>(workload.structs[*sequences_struct_index].data.data()) : nullptr;
        
        for(std::size_t t = 0; t < type_count; t++) {
            auto &type = types[t];
            std::size_t sequence_index = type.sequence_index;
            std::size_t sprite_count = 0;
            std::size_t sequence_offset = 0;
            
            // Make sure it's not out-of-bounds
            if(type.sequence_index >= sequence_count) {
                if(!workload.disable_error_checking) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence index #%zu of bitmap %s.%s for type #%zu is out of bounds (%zu >= %zu)", sequence_index, File::halo_path_to_preferred_path(sprite_plate_tag.path).c_str(), HEK::tag_fourcc_to_extension(sprite_plate_tag.tag_fourcc), sequence_index, sequence_index, sequence_count);
                    throw InvalidTagDataException();
                }
            }
            else {
                // Make sure we don't have too many sprites (since it's only an 8 bit field)
                sprite_count = sequences[sequence_index].sprites.count.read();
                static const constexpr std::size_t MAX_SPRITE_COUNT = static_cast<decltype(type.sprite_count)>(~0);
                
                // Increment this
                for(std::size_t s = 0; s <= sequence_index; s++) {
                    sequence_offset += sequences[s].sprites.count.read();
                }
            
                // TODO: Note these limitations in the definitions
                if(!workload.disable_error_checking) {
                    if(sprite_count > MAX_SPRITE_COUNT) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence index #%zu of bitmap %s.%s for type #%zu exceeds the maximum number of sprites allowed for detail object collections (%zu > %zu)", sequence_index, File::halo_path_to_preferred_path(sprite_plate_tag.path).c_str(), HEK::tag_fourcc_to_extension(sprite_plate_tag.tag_fourcc), sequence_index, static_cast<std::size_t>(sprite_count), MAX_SPRITE_COUNT);
                        throw InvalidTagDataException();
                    }
                    if(sequence_offset > MAX_SPRITE_COUNT) {
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence index #%zu of bitmap %s.%s for type #%zu has a sprite offset which exceeds the maximum number of sprites allowed for detail object collections (%zu > %zu)", sequence_index, File::halo_path_to_preferred_path(sprite_plate_tag.path).c_str(), HEK::tag_fourcc_to_extension(sprite_plate_tag.tag_fourcc), sequence_offset, static_cast<std::size_t>(sprite_count), MAX_SPRITE_COUNT);
                        throw InvalidTagDataException();
                    }
                }
            }
            
            // Punch it
            type.sprite_count = static_cast<decltype(type.sprite_count)>(sprite_count);
            type.sprite_count = static_cast<decltype(type.sprite_count)>(sequence_offset);
        }
    }
}
