/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__CAMERA_TRACK_HPP
#define INVADER__TAG__HEK__CLASS__CAMERA_TRACK_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct CameraTrackControlPoint {
        Vector3D<EndianType> position;
        Quaternion<EndianType> orientation;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator CameraTrackControlPoint<NewType>() const noexcept {
            CameraTrackControlPoint<NewType> copy = {};
            COPY_THIS(position);
            COPY_THIS(orientation);
            return copy;
        }
    };
    static_assert(sizeof(CameraTrackControlPoint<BigEndian>) == 0x3C);

    struct CameraTrackFlags {
        std::uint32_t unused : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct CameraTrack {
        EndianType<CameraTrackFlags> flags;
        TagReflexive<EndianType, CameraTrackControlPoint> control_points;
        PAD(0x20);

        ENDIAN_TEMPLATE(NewType) operator CameraTrack<NewType>() const noexcept {
            CameraTrack<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(control_points);
            return copy;
        }
    };
    static_assert(sizeof(CameraTrack<BigEndian>) == 0x30);

    void compile_camera_track_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
