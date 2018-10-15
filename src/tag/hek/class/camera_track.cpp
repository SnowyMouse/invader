/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "camera_track.hpp"

namespace Invader::HEK {
    void compile_camera_track_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(CameraTrack);
        ADD_REFLEXIVE(tag.control_points);
        FINISH_COMPILE
    }
}
