/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__HUD_NUMBER_HPP
#define INVADER__TAG__HEK__CLASS__HUD_NUMBER_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct HUDNumber {
        TagDependency<EndianType> digits_bitmap; // bitmap
        std::int8_t bitmap_digit_width;
        std::int8_t screen_digit_width;
        std::int8_t x_offset;
        std::int8_t y_offset;
        std::int8_t decimal_point_width;
        std::int8_t colon_width;
        PAD(0x2);
        PAD(0x4C);

        ENDIAN_TEMPLATE(NewType) operator HUDNumber<NewType>() const noexcept {
            HUDNumber<NewType> copy = {};
            COPY_THIS(digits_bitmap);
            COPY_THIS(bitmap_digit_width);
            COPY_THIS(screen_digit_width);
            COPY_THIS(x_offset);
            COPY_THIS(y_offset);
            COPY_THIS(decimal_point_width);
            COPY_THIS(colon_width);
            return copy;
        }
    };
    static_assert(sizeof(HUDNumber<BigEndian>) == 0x64);

    void compile_hud_number_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
