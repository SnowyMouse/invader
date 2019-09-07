/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "../../../hek/constants.hpp"

#include "sound.hpp"

namespace Invader::HEK {
    void compile_sound_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Sound);
        DEFAULT_VALUE(tag.random_pitch_bounds.from, 1.0f);
        DEFAULT_VALUE(tag.random_pitch_bounds.to, 1.0f);
        DEFAULT_VALUE(tag.inner_cone_angle, 2 * static_cast<float>(HALO_PI));
        DEFAULT_VALUE(tag.outer_cone_angle, 2 * static_cast<float>(HALO_PI));
        DEFAULT_VALUE(tag.outer_cone_gain, 1.0f);
        DEFAULT_VALUE(tag.random_gain_modifier, 1.0f);

        if(tag.one_skip_fraction_modifier == 0.0f && tag.zero_skip_fraction_modifier == 0.0f) {
            tag.one_skip_fraction_modifier = 1.0f;
            tag.zero_skip_fraction_modifier = 1.0f;
        }

        if(tag.one_gain_modifier == 0.0f && tag.zero_gain_modifier == 0.0f) {
            tag.one_gain_modifier = 1.0f;
            tag.zero_gain_modifier = 1.0f;
        }

        if(tag.zero_pitch_modifier == 0.0f && tag.one_pitch_modifier == 0.0f) {
            tag.one_pitch_modifier = 1.0f;
            tag.zero_pitch_modifier = 1.0f;
        }

        // I don't know why this is a thing but it is
        tag.maximum_bend_per_second = std::pow(tag.maximum_bend_per_second, 1.0f / TICK_RATE);

        // I don't know what this is either. Sorry.
        tag.unknown_ffffffff_0 = 0xFFFFFFFF;
        tag.unknown_ffffffff_1 = 0xFFFFFFFF;

        ADD_DEPENDENCY_ADJUST_SIZES(tag.promotion_sound);
        ADD_REFLEXIVE_START(tag.pitch_ranges) {
            DEFAULT_VALUE(reflexive.natural_pitch, 1.0f);
            DEFAULT_VALUE(reflexive.bend_bounds.to, 1.0f);
            reflexive.playback_rate = 1.0f / reflexive.natural_pitch;

            // I don't know what this is. Sorry.
            tag.unknown_ffffffff_0 = 0xFFFFFFFF;
            tag.unknown_ffffffff_1 = 0xFFFFFFFF;

            ADD_REFLEXIVE_START(reflexive.permutations) {
                reflexive.samples.pointer = static_cast<std::int32_t>(compiled.data.size());

                std::size_t samples_size = reflexive.samples.size;
                reflexive.samples.file_offset = static_cast<std::int32_t>(compiled.asset_data.size());
                compiled.asset_data.insert(compiled.asset_data.end(), data, data + samples_size);
                INCREMENT_DATA_PTR(samples_size)
                reflexive.samples.pointer = 0;
                reflexive.samples_pointer = -1;

                std::size_t mouth_data_size = reflexive.mouth_data.size;
                ADD_POINTER_FROM_INT32(reflexive.mouth_data.pointer, compiled.data.size());
                compiled.data.insert(compiled.data.end(), data, data + mouth_data_size);
                INCREMENT_DATA_PTR(mouth_data_size)

                std::size_t subtitle_data_size = reflexive.subtitle_data.size;
                // ADD_POINTER_FROM_INT32(reflexive.subtitle_data.pointer, compiled.data.size());
                compiled.data.insert(compiled.data.end(), data, data + subtitle_data_size);
                INCREMENT_DATA_PTR(subtitle_data_size)

                DEFAULT_VALUE(reflexive.gain, 1.0f);
                PAD_32_BIT
            } ADD_REFLEXIVE_END
        } ADD_REFLEXIVE_END
        FINISH_COMPILE
    }
}
