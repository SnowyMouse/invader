/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__DETAIL_OBJECT_COLLECTION_HPP
#define INVADER__TAG__HEK__CLASS__DETAIL_OBJECT_COLLECTION_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum DetailObjectCollectionType : TagEnum {
        DETAIL_OBJECT_COLLECTION_TYPE_SCREEN_FACING,
        DETAIL_OBJECT_COLLECTION_TYPE_VIEWER_FACING
    };

    struct DetailObjectCollectionTypeFlags {
        std::uint8_t unused : 2;
        std::uint8_t interpolate_color_in_hsv : 1;
        std::uint8_t more_colors : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct DetailObjectCollectionObjectType {
        TagString name;
        std::uint8_t sequence_index;
        DetailObjectCollectionTypeFlags flags;
        PAD(0x1);
        std::uint8_t sprite_count;
        EndianType<Fraction> color_override_factor;
        PAD(0x8);
        EndianType<float> near_fade_distance;
        EndianType<float> far_fade_distance;
        EndianType<float> size;
        PAD(0x4);
        ColorRGB<EndianType> minimum_color;
        ColorRGB<EndianType> maximum_color;
        EndianType<ColorARGBInt> ambient_color;
        PAD(0x4);

        ENDIAN_TEMPLATE(NewType) operator DetailObjectCollectionObjectType<NewType>() const noexcept {
            DetailObjectCollectionObjectType<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(sequence_index);
            COPY_THIS(flags);
            COPY_THIS(sprite_count);
            COPY_THIS(color_override_factor);
            COPY_THIS(near_fade_distance);
            COPY_THIS(far_fade_distance);
            COPY_THIS(size);
            COPY_THIS(minimum_color);
            COPY_THIS(maximum_color);
            COPY_THIS(ambient_color);
            return copy;
        }
    };
    static_assert(sizeof(DetailObjectCollectionObjectType<NativeEndian>) == 0x60);

    ENDIAN_TEMPLATE(EndianType) struct DetailObjectCollection {
        EndianType<DetailObjectCollectionType> collection_type;
        PAD(0x2);
        EndianType<float> global_z_offset;
        PAD(0x2C);
        TagDependency<EndianType> sprite_plate; // .bitmap
        TagReflexive<EndianType, DetailObjectCollectionObjectType> types;
        PAD(0x30);

        ENDIAN_TEMPLATE(NewType) operator DetailObjectCollection<NewType>() const noexcept {
            DetailObjectCollection<NewType> copy = {};
            COPY_THIS(collection_type);
            COPY_THIS(global_z_offset);
            COPY_THIS(sprite_plate);
            COPY_THIS(types);
            return copy;
        }
    };
    static_assert(sizeof(DetailObjectCollection<NativeEndian>) == 0x80);

    void compile_detail_object_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
