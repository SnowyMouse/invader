/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum MeterInterpolateColors : TagEnum {
        METER_INTERPOLATE_COLORS_LINEARLY,
        METER_INTERPOLATE_COLORS_FASTER_NEAR_EMPTY,
        METER_INTERPOLATE_COLORS_FASTER_NEAR_FULL,
        METER_INTERPOLATE_COLORS_THROUGH_RANDOM_NOISE
    };

    enum MeterAnchorColors : TagEnum {
        METER_ANCHOR_COLORS_AT_BOTH_ENDS,
        METER_ANCHOR_COLORS_AT_EMPTY,
        METER_ANCHOR_COLORS_AT_FULL
    };

    struct MeterFlags {
        std::uint32_t unused : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Meter {
        EndianType<MeterFlags> flags;
        TagDependency<EndianType> stencil_bitmaps; // bitmap
        TagDependency<EndianType> source_bitmap; // bitmap
        EndianType<std::uint16_t> stencil_sequence_index;
        EndianType<std::uint16_t> source_sequence_index;
        PAD(0x10);
        PAD(0x4);
        EndianType<MeterInterpolateColors> interpolate_colors;
        EndianType<MeterAnchorColors> anchor_colors;
        PAD(0x8);
        ColorARGB<EndianType> empty_color;
        ColorARGB<EndianType> full_color;
        PAD(0x14);
        EndianType<float> unmask_distance;
        EndianType<float> mask_distance;
        PAD(0x14);
        TagDataOffset<EndianType> encoded_stencil;

        ENDIAN_TEMPLATE(NewType) operator Meter<NewType>() const noexcept {
            Meter<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(stencil_bitmaps);
            COPY_THIS(source_bitmap);
            COPY_THIS(stencil_sequence_index);
            COPY_THIS(source_sequence_index);
            COPY_THIS(interpolate_colors);
            COPY_THIS(anchor_colors);
            COPY_THIS(empty_color);
            COPY_THIS(full_color);
            COPY_THIS(unmask_distance);
            COPY_THIS(mask_distance);
            COPY_THIS(encoded_stencil);
            return copy;
        }
    };
    static_assert(sizeof(Meter<BigEndian>) == 0xAC);

    void compile_meter_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
