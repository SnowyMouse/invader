/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__ANTENNA_HPP
#define INVADER__TAG__HEK__CLASS__ANTENNA_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct AntennaVertex {
        EndianType<Fraction> spring_strength_coefficient;
        PAD(0x18);
        Euler2D<EndianType> angles;
        EndianType<float> length;
        EndianType<std::int16_t> sequence_index;
        PAD(0x2);
        ColorARGB<EndianType> color;
        ColorARGB<EndianType> lod_color;
        PAD(0x28);
        Point3D<LittleEndian> offset;

        ENDIAN_TEMPLATE(NewType) operator AntennaVertex<NewType>() const noexcept {
            AntennaVertex<NewType> copy = {};
            COPY_THIS(spring_strength_coefficient);
            COPY_THIS(angles);
            COPY_THIS(length);
            COPY_THIS(sequence_index);
            COPY_THIS(color);
            COPY_THIS(lod_color);
            COPY_THIS(offset);
            return copy;
        }
    };
    static_assert(sizeof(AntennaVertex<BigEndian>) == 0x80);

    ENDIAN_TEMPLATE(EndianType) struct Antenna {
        TagString attachment_marker_name;
        TagDependency<EndianType> bitmaps; // bitmap
        TagDependency<EndianType> physics; // point_physics
        PAD(0x50);
        EndianType<Fraction> spring_strength_coefficient;
        EndianType<float> falloff_pixels;
        EndianType<float> cutoff_pixels;
        EndianType<float> length;
        PAD(0x24);
        TagReflexive<EndianType, AntennaVertex> vertices;

        ENDIAN_TEMPLATE(NewType) operator Antenna<NewType>() const noexcept {
            Antenna<NewType> copy = {};
            COPY_THIS(attachment_marker_name);
            COPY_THIS(bitmaps);
            COPY_THIS(physics);
            COPY_THIS(spring_strength_coefficient);
            COPY_THIS(falloff_pixels);
            COPY_THIS(cutoff_pixels);
            COPY_THIS(length);
            COPY_THIS(vertices);
            return copy;
        }
    };
    static_assert(sizeof(Antenna<BigEndian>) == 0xD0);

    void compile_antenna_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
