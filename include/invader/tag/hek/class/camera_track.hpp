// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__CAMERA_TRACK_HPP
#define INVADER__TAG__HEK__CLASS__CAMERA_TRACK_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"
#include "bitfield.hpp"

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

    ENDIAN_TEMPLATE(EndianType) struct CameraTrack {
        EndianType<IsUnusedFlag> flags;
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
