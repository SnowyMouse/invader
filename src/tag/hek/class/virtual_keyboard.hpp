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
    enum VirtualKeyboardKeyboardKey : TagEnum {
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_1,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_2,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_3,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_4,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_5,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_6,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_7,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_8,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_9,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_0,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_A,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_B,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_C,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_D,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_E,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_F,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_G,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_H,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_I,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_J,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_K,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_L,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_M,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_N,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_O,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_P,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_Q,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_R,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_S,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_T,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_U,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_V,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_W,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_X,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_Y,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_Z,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_DONE,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_SHIFT,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_CAPS_LOCK,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_SYMBOLS,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_BACKSPACE,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_LEFT,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_RIGHT,
        VIRTUAL_KEYBOARD_KEYBOARD_KEY_SPACE
    };

    ENDIAN_TEMPLATE(EndianType) struct VirtualKeyboardVirtualKey {
        EndianType<VirtualKeyboardKeyboardKey> keyboard_key;
        EndianType<std::int16_t> lowercase_character;
        EndianType<std::int16_t> shift_character;
        EndianType<std::int16_t> caps_character;
        EndianType<std::int16_t> symbols_character;
        EndianType<std::int16_t> shift_caps_character;
        EndianType<std::int16_t> shift_symbols_character;
        EndianType<std::int16_t> caps_symbols_character;
        TagDependency<EndianType> unselected_background_bitmap; // bitmap
        TagDependency<EndianType> selected_background_bitmap; // bitmap
        TagDependency<EndianType> active_background_bitmap; // bitmap
        TagDependency<EndianType> sticky_background_bitmap; // bitmap

        ENDIAN_TEMPLATE(NewType) operator VirtualKeyboardVirtualKey<NewType>() const noexcept {
            VirtualKeyboardVirtualKey<NewType> copy;
            COPY_THIS(keyboard_key);
            COPY_THIS(lowercase_character);
            COPY_THIS(shift_character);
            COPY_THIS(caps_character);
            COPY_THIS(symbols_character);
            COPY_THIS(shift_caps_character);
            COPY_THIS(shift_symbols_character);
            COPY_THIS(caps_symbols_character);
            COPY_THIS(unselected_background_bitmap);
            COPY_THIS(selected_background_bitmap);
            COPY_THIS(active_background_bitmap);
            COPY_THIS(sticky_background_bitmap);
            return copy;
        }
    };
    static_assert(sizeof(VirtualKeyboardVirtualKey<BigEndian>) == 0x50);

    ENDIAN_TEMPLATE(EndianType) struct VirtualKeyboard {
        TagDependency<EndianType> display_font; // font
        TagDependency<EndianType> background_bitmap; // bitmap
        TagDependency<EndianType> special_key_labels_string_list; // unicode_string
        TagReflexive<EndianType, VirtualKeyboardVirtualKey> virtual_keys;

        ENDIAN_TEMPLATE(NewType) operator VirtualKeyboard<NewType>() const noexcept {
            VirtualKeyboard<NewType> copy;
            COPY_THIS(display_font);
            COPY_THIS(background_bitmap);
            COPY_THIS(special_key_labels_string_list);
            COPY_THIS(virtual_keys);
            return copy;
        }
    };
    static_assert(sizeof(VirtualKeyboard<BigEndian>) == 0x3C);

    void compile_virtual_keyboard_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
