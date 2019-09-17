/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"

#include "sound_looping.hpp"

namespace Invader::HEK {
    void compile_sound_looping_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(SoundLooping)
        if(tag.zero_detail_sound_period == 0.0f && tag.one_detail_sound_period == 0.0f) {
            tag.zero_detail_sound_period = 1.0f;
            tag.one_detail_sound_period = 1.0f;
        }
        DEFAULT_VALUE(tag.zero_detail_unknown_floats[0], 1.0f);
        DEFAULT_VALUE(tag.zero_detail_unknown_floats[1], 1.0f);
        DEFAULT_VALUE(tag.one_detail_unknown_floats[0], 1.0f);
        DEFAULT_VALUE(tag.one_detail_unknown_floats[1], 1.0f);
        DEFAULT_VALUE(tag.unknown_int, -1);
        DEFAULT_VALUE(tag.unknown_float, 15.0f);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.continuous_damage_effect);
        ADD_REFLEXIVE_START(tag.tracks) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.start);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.loop);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.end);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.alternate_loop);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.alternate_end);
            DEFAULT_VALUE(reflexive.gain, 1.0f);
        } ADD_REFLEXIVE_END
        ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.detail_sounds, sound);
        FINISH_COMPILE
    }
}
