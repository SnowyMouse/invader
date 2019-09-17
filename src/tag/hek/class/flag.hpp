/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__FLAG_HPP
#define INVADER__TAG__HEK__CLASS__FLAG_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct FlagAttachmentPoint {
        EndianType<std::int16_t> height_to_next_attachment;
        PAD(0x2);
        PAD(0x10);
        TagString marker_name;

        ENDIAN_TEMPLATE(NewType) operator FlagAttachmentPoint<NewType>() const noexcept {
            FlagAttachmentPoint<NewType> copy = {};
            COPY_THIS(height_to_next_attachment);
            COPY_THIS(marker_name);
            return copy;
        }
    };

    enum FlagTrailingEdgeShape : TagEnum {
        FLAG_TRAILING_EDGE_SHAPE_FLAT,
        FLAG_TRAILING_EDGE_SHAPE_CONCAVE_TRIANGULAR,
        FLAG_TRAILING_EDGE_SHAPE_CONVEX_TRIANGULAR,
        FLAG_TRAILING_EDGE_SHAPE_TRAPEZOID_SHORT_TOP,
        FLAG_TRAILING_EDGE_SHAPE_TRAPEZOID_SHORT_BOTTOM
    };

    enum AttachedEdgeShape : TagEnum {
        FLAG_ATTACHED_EDGE_SHAPE_FLAT,
        FLAG_ATTACHED_EDGE_SHAPE_CONCAVE_TRIANGULAR
    };

    struct FlagFlags {
        std::uint32_t unused : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Flag {
        EndianType<FlagFlags> flags;
        EndianType<FlagTrailingEdgeShape> trailing_edge_shape;
        EndianType<std::int16_t> trailing_edge_shape_offset;
        EndianType<AttachedEdgeShape> attached_edge_shape;
        PAD(0x2);
        EndianType<std::int16_t> width;
        EndianType<std::int16_t> height;
        EndianType<float> cell_width;
        EndianType<float> cell_height;
        TagDependency<EndianType> red_flag_shader; // shader
        TagDependency<EndianType> physics; // point_physics
        EndianType<float> wind_noise;
        PAD(0x8);
        TagDependency<EndianType> blue_flag_shader; // shader
        TagReflexive<EndianType, FlagAttachmentPoint> attachment_points;

        ENDIAN_TEMPLATE(NewType) operator Flag<NewType>() const noexcept {
            Flag<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(trailing_edge_shape);
            COPY_THIS(trailing_edge_shape_offset);
            COPY_THIS(attached_edge_shape);
            COPY_THIS(width);
            COPY_THIS(height);
            COPY_THIS(cell_width);
            COPY_THIS(cell_height);
            COPY_THIS(red_flag_shader);
            COPY_THIS(physics);
            COPY_THIS(wind_noise);
            COPY_THIS(blue_flag_shader);
            COPY_THIS(attachment_points);
            return copy;
        }
    };
    static_assert(sizeof(Flag<BigEndian>) == 0x60);

    void compile_flag_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
