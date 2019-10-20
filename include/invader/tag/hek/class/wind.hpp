// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__WIND_HPP
#define INVADER__TAG__HEK__CLASS__WIND_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct Wind {
        Bounds<EndianType<float>> velocity;
        Euler2D<EndianType> variation_area;
        EndianType<float> local_variation_weight;
        EndianType<float> local_variation_rate;
        EndianType<float> damping;
        PAD(0x24);

        ENDIAN_TEMPLATE(OtherType) operator Wind<OtherType>() const noexcept {
            Wind<OtherType> copy = {};
            COPY_THIS(velocity);
            COPY_THIS(variation_area);
            COPY_THIS(local_variation_weight);
            COPY_THIS(local_variation_rate);
            COPY_THIS(damping);
            return copy;
        }
    };
    static_assert(sizeof(Wind<BigEndian>) == 0x40);

    void compile_wind_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
