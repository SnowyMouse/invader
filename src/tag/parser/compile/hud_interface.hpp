// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__PARSER__COMPILE__HUD_INTERFACE_HPP
#define INVADER__PARSER__COMPILE__HUD_INTERFACE_HPP

#include <cstddef>
#include <invader/tag/parser/parser.hpp>

#define CHECK_BITMAP_SEQUENCE(workload, bitmap_tag, sequence_index, name) { \
    if((!workload.disable_recursion && !workload.disable_error_checking) && (!bitmap_tag.tag_id.is_null())) { \
        std::size_t sequence_count; \
        const BitmapGroupSequence::struct_little *sequences; \
        char bitmap_tag_path[256]; \
        HEK::BitmapType bitmap_type; \
        get_sequence_data(workload, bitmap_tag.tag_id, sequence_count, sequences, bitmap_tag_path, sizeof(bitmap_tag_path), bitmap_type); \
        if(sequence_index >= sequence_count) { \
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in '%s' referenced by %s is out of bounds (>= %zu)", static_cast<std::size_t>(sequence_index), bitmap_tag_path, name, sequence_count); \
            throw InvalidTagDataException(); \
        } \
        else { \
            auto &sequence = sequences[sequence_index]; \
            if(bitmap_type == HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.sprites.count == 0) { \
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in '%s' referenced in %s has 0 sprites", static_cast<std::size_t>(sequence_index), name, bitmap_tag_path); \
                throw InvalidTagDataException(); \
            } \
            else if(bitmap_type != HEK::BitmapType::BITMAP_TYPE_SPRITES && sequence.bitmap_count == 0) { \
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sequence #%zu in '%s' referenced in %s has 0 bitmaps", static_cast<std::size_t>(sequence_index), name, bitmap_tag_path); \
                throw InvalidTagDataException(); \
            } \
        } \
    } \
}

namespace Invader::Parser {
    void get_sequence_data(const Invader::BuildWorkload &workload, const HEK::TagID &tag_id, std::size_t &sequence_count, const BitmapGroupSequence::struct_little *&sequences, char *bitmap_tag_path, std::size_t bitmap_tag_path_size, HEK::BitmapType &bitmap_type);
}

#endif
