/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SOUND_ENVIRONMENT_HPP
#define INVADER__TAG__HEK__CLASS__SOUND_ENVIRONMENT_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct SoundEnvironment {
        EndianType<std::int32_t> unknown;
        EndianType<std::int16_t> priority;
        PAD(0x2);
        EndianType<Fraction> room_intensity;
        EndianType<Fraction> room_intensity_hf;
        EndianType<float> room_rolloff;
        EndianType<float> decay_time;
        EndianType<float> decay_hf_ratio;
        EndianType<Fraction> reflections_intensity;
        EndianType<float> reflections_delay;
        EndianType<Fraction> reverb_intensity;
        EndianType<float> reverb_delay;
        EndianType<float> diffusion;
        EndianType<float> density;
        EndianType<float> hf_reference;
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator SoundEnvironment<NewType>() const noexcept {
            SoundEnvironment<NewType> copy = {};
            COPY_THIS(unknown);
            COPY_THIS(priority);
            COPY_THIS(room_intensity);
            COPY_THIS(room_intensity_hf);
            COPY_THIS(room_rolloff);
            COPY_THIS(decay_time);
            COPY_THIS(decay_hf_ratio);
            COPY_THIS(reflections_intensity);
            COPY_THIS(reflections_delay);
            COPY_THIS(reverb_intensity);
            COPY_THIS(reverb_delay);
            COPY_THIS(diffusion);
            COPY_THIS(density);
            COPY_THIS(hf_reference);
            return copy;
        }
    };
    static_assert(sizeof(SoundEnvironment<BigEndian>) == 0x48);

    void compile_sound_environment_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
