/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__DEVICE_LIGHT_FIXTURE_HPP
#define INVADER__TAG__HEK__CLASS__DEVICE_LIGHT_FIXTURE_HPP

#include "device.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct DeviceLightFixture : Device<EndianType> {
        PAD(0x40);
        ENDIAN_TEMPLATE(NewType) operator DeviceLightFixture<NewType>() const noexcept {
            DeviceLightFixture<NewType> copy = {};
            COPY_DEVICE_DATA
            return copy;
        }
    };
    static_assert(sizeof(DeviceLightFixture<NativeEndian>) == 0x2D0);

    void compile_device_light_fixture_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
