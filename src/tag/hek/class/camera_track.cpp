// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "camera_track.hpp"

namespace Invader::HEK {
    void compile_camera_track_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(CameraTrack);
        ADD_REFLEXIVE(tag.control_points);
        FINISH_COMPILE
    }
}
