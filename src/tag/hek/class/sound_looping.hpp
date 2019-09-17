/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SOUND_LOOPING_HPP
#define INVADER__TAG__HEK__CLASS__SOUND_LOOPING_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    struct SoundLoopingTrackFlags {
        std::uint32_t fade_in_at_start : 1;
        std::uint32_t fade_out_at_stop : 1;
        std::uint32_t fade_in_alternate : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct SoundLoopingTrack {
        EndianType<SoundLoopingTrackFlags> flags;
        EndianType<Fraction> gain;
        EndianType<float> fade_in_duration;
        EndianType<float> fade_out_duration;
        PAD(0x20);
        TagDependency<EndianType> start; // sound
        TagDependency<EndianType> loop; // sound
        TagDependency<EndianType> end; // sound
        PAD(0x20);
        TagDependency<EndianType> alternate_loop; // sound
        TagDependency<EndianType> alternate_end; // sound

        ENDIAN_TEMPLATE(NewType) operator SoundLoopingTrack<NewType>() const noexcept {
            SoundLoopingTrack<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(gain);
            COPY_THIS(fade_in_duration);
            COPY_THIS(fade_out_duration);
            COPY_THIS(start);
            COPY_THIS(loop);
            COPY_THIS(end);
            COPY_THIS(alternate_loop);
            COPY_THIS(alternate_end);
            return copy;
        }
    };
    static_assert(sizeof(SoundLoopingTrack<BigEndian>) == 0xA0);

    struct SoundLoopingDetailFlags {
        std::uint32_t don_t_play_with_alternate : 1;
        std::uint32_t don_t_play_without_alternate : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct SoundLoopingDetail {
        TagDependency<EndianType> sound; // sound
        Bounds<EndianType<float>> random_period_bounds;
        EndianType<Fraction> gain;
        EndianType<SoundLoopingDetailFlags> flags;
        PAD(0x30);
        Bounds<EndianType<Angle>> yaw_bounds;
        Bounds<EndianType<Angle>> pitch_bounds;
        Bounds<EndianType<float>> distance_bounds;

        ENDIAN_TEMPLATE(NewType) operator SoundLoopingDetail<NewType>() const noexcept {
            SoundLoopingDetail<NewType> copy = {};
            COPY_THIS(sound);
            COPY_THIS(random_period_bounds);
            COPY_THIS(flags);
            COPY_THIS(yaw_bounds);
            COPY_THIS(pitch_bounds);
            COPY_THIS(distance_bounds);
            return copy;
        }
    };
    static_assert(sizeof(SoundLoopingDetail<BigEndian>) == 0x68);

    struct SoundLoopingFlags {
        std::uint32_t deafening_to_ais : 1;
        std::uint32_t not_a_loop : 1;
        std::uint32_t stops_music : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct SoundLooping {
        EndianType<SoundLoopingFlags> flags;
        EndianType<float> zero_detail_sound_period;
        EndianType<float> zero_detail_unknown_floats[2];
        EndianType<float> one_detail_sound_period;
        EndianType<float> one_detail_unknown_floats[2];
        EndianType<std::int32_t> unknown_int;
        EndianType<float> unknown_float;
        PAD(0x8);
        TagDependency<EndianType> continuous_damage_effect; // continuous_damage_effect
        TagReflexive<EndianType, SoundLoopingTrack> tracks;
        TagReflexive<EndianType, SoundLoopingDetail> detail_sounds;

        ENDIAN_TEMPLATE(NewType) operator SoundLooping<NewType>() const noexcept {
            SoundLooping<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(zero_detail_sound_period);
            COPY_THIS_ARRAY(zero_detail_unknown_floats);
            COPY_THIS(one_detail_sound_period);
            COPY_THIS_ARRAY(one_detail_unknown_floats);
            COPY_THIS(continuous_damage_effect);
            COPY_THIS(unknown_int);
            COPY_THIS(unknown_float);
            COPY_THIS(tracks);
            COPY_THIS(detail_sounds);
            return copy;
        }
    };
    static_assert(sizeof(SoundLooping<BigEndian>) == 0x54);

    void compile_sound_looping_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
