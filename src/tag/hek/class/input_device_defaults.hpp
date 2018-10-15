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
    enum InputDeviceDefaultsDeviceType : TagEnum {
        INPUT_DEVICE_DEFAULTS_DEVICE_TYPE_MOUSE_AND_KEYBOARD,
        INPUT_DEVICE_DEFAULTS_DEVICE_TYPE_JOYSTICKS_GAMEPADS_ETC,
        INPUT_DEVICE_DEFAULTS_DEVICE_TYPE_FULL_PROFILE_DEFINITION
    };

    ENDIAN_TEMPLATE(EndianType) struct InputDeviceDefaults {
        EndianType<InputDeviceDefaultsDeviceType> device_type;
        EndianType<std::int16_t> unused;
        TagDataOffset<EndianType> device_id;
        TagDataOffset<EndianType> profile;

        ENDIAN_TEMPLATE(NewType) operator InputDeviceDefaults<NewType>() const noexcept {
            InputDeviceDefaults<NewType> copy;
            COPY_THIS(device_type);
            COPY_THIS(unused);
            COPY_THIS(device_id);
            COPY_THIS(profile);
            return copy;
        }
    };
    static_assert(sizeof(InputDeviceDefaults<BigEndian>) == 0x2C);

    void compile_input_device_defaults_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
