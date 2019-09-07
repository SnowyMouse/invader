/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__SOUND_HPP
#define INVADER__TAG__HEK__CLASS__SOUND_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    enum SoundCompression : TagEnum {
        SOUND_COMPRESSION_NONE,
        SOUND_COMPRESSION_XBOX_ADPCM,
        SOUND_COMPRESSION_IMA_ADPCM,
        SOUND_COMPRESSION_OGG
    };

    enum SoundClass : TagEnum {
        SOUND_CLASS_PROJECTILE_IMPACT,
        SOUND_CLASS_PROJECTILE_DETONATION,
        SOUND_CLASS_UNUSED,
        SOUND_CLASS_UNUSED_1,
        SOUND_CLASS_WEAPON_FIRE,
        SOUND_CLASS_WEAPON_READY,
        SOUND_CLASS_WEAPON_RELOAD,
        SOUND_CLASS_WEAPON_EMPTY,
        SOUND_CLASS_WEAPON_CHARGE,
        SOUND_CLASS_WEAPON_OVERHEAT,
        SOUND_CLASS_WEAPON_IDLE,
        SOUND_CLASS_UNUSED_2,
        SOUND_CLASS_UNUSED_3,
        SOUND_CLASS_OBJECT_IMPACTS,
        SOUND_CLASS_PARTICLE_IMPACTS,
        SOUND_CLASS_SLOW_PARTICLE_IMPACTS,
        SOUND_CLASS_UNUSED_4,
        SOUND_CLASS_UNUSED_5,
        SOUND_CLASS_UNIT_FOOTSTEPS,
        SOUND_CLASS_UNIT_DIALOG,
        SOUND_CLASS_UNUSED_6,
        SOUND_CLASS_UNUSED_7,
        SOUND_CLASS_VEHICLE_COLLISION,
        SOUND_CLASS_VEHICLE_ENGINE,
        SOUND_CLASS_UNUSED_8,
        SOUND_CLASS_UNUSED_9,
        SOUND_CLASS_DEVICE_DOOR,
        SOUND_CLASS_DEVICE_FORCE_FIELD,
        SOUND_CLASS_DEVICE_MACHINERY,
        SOUND_CLASS_DEVICE_NATURE,
        SOUND_CLASS_DEVICE_COMPUTERS,
        SOUND_CLASS_UNUSED_10,
        SOUND_CLASS_MUSIC,
        SOUND_CLASS_AMBIENT_NATURE,
        SOUND_CLASS_AMBIENT_MACHINERY,
        SOUND_CLASS_AMBIENT_COMPUTERS,
        SOUND_CLASS_UNUSED_11,
        SOUND_CLASS_UNUSED_12,
        SOUND_CLASS_UNUSED_13,
        SOUND_CLASS_FIRST_PERSON_DAMAGE,
        SOUND_CLASS_UNUSED_14,
        SOUND_CLASS_UNUSED_15,
        SOUND_CLASS_UNUSED_16,
        SOUND_CLASS_UNUSED_17,
        SOUND_CLASS_SCRIPTED_DIALOG_PLAYER,
        SOUND_CLASS_SCRIPTED_EFFECT,
        SOUND_CLASS_SCRIPTED_DIALOG_OTHER,
        SOUND_CLASS_SCRIPTED_DIALOG_FORCE_UNSPATIALIZED,
        SOUND_CLASS_UNUSED_18,
        SOUND_CLASS_UNUSED_19,
        SOUND_CLASS_GAME_EVENT
    };

    enum SoundSampleRate : TagEnum {
        SOUND_SAMPLE_RATE_22KHZ,
        SOUND_SAMPLE_RATE_44KHZ
    };
    enum SoundEncoding : TagEnum {
        SOUND_ENCODING_MONO,
        SOUND_ENCODING_STEREO
    };

    ENDIAN_TEMPLATE(EndianType) struct SoundPermutation {
        TagString name;
        EndianType<Fraction> skip_fraction;
        EndianType<Fraction> gain;
        EndianType<SoundCompression> compression;
        EndianType<std::int16_t> next_permutation_index;
        LittleEndian<std::uint32_t> samples_pointer;
        PAD(0x4);
        EndianType<TagID> tag_id_0;
        LittleEndian<std::uint32_t> sample_count;
        EndianType<TagID> tag_id_1;
        TagDataOffset<EndianType> samples;
        TagDataOffset<EndianType> mouth_data;
        TagDataOffset<EndianType> subtitle_data;

        ENDIAN_TEMPLATE(NewType) operator SoundPermutation<NewType>() const noexcept {
            SoundPermutation<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(skip_fraction);
            COPY_THIS(gain);
            COPY_THIS(compression);
            COPY_THIS(next_permutation_index);
            COPY_THIS(samples_pointer);
            COPY_THIS(tag_id_0);
            COPY_THIS(sample_count);
            COPY_THIS(tag_id_1);
            COPY_THIS(samples);
            COPY_THIS(mouth_data);
            COPY_THIS(subtitle_data);
            return copy;
        }
    };
    static_assert(sizeof(SoundPermutation<BigEndian>) == 0x7C);

    ENDIAN_TEMPLATE(EndianType) struct SoundPitchRange {
        TagString name;
        EndianType<float> natural_pitch;
        Bounds<EndianType<float>> bend_bounds;
        EndianType<std::int16_t> actual_permutation_count;
        PAD(0x2);
        EndianType<float> playback_rate;
        LittleEndian<std::uint32_t> unknown_ffffffff_0;
        LittleEndian<std::uint32_t> unknown_ffffffff_1;
        TagReflexive<EndianType, SoundPermutation> permutations;

        ENDIAN_TEMPLATE(NewType) operator SoundPitchRange<NewType>() const noexcept {
            SoundPitchRange<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(natural_pitch);
            COPY_THIS(bend_bounds);
            COPY_THIS(actual_permutation_count);
            COPY_THIS(permutations);
            COPY_THIS(playback_rate);
            COPY_THIS(unknown_ffffffff_0);
            COPY_THIS(unknown_ffffffff_1);
            return copy;
        }
    };
    static_assert(sizeof(SoundPitchRange<BigEndian>) == 0x48);

    struct SoundFlags {
        std::uint32_t fit_to_adpcm_blocksize : 1;
        std::uint32_t split_long_sound_into_permutations : 1;
        std::uint32_t reserved : 1;
        std::uint32_t reserved_1 : 1;
        std::uint32_t reserved_2 : 1;
        std::uint32_t reserved_3 : 1;
        std::uint32_t reserved_4 : 1;
        std::uint32_t reserved_5 : 1;
        std::uint32_t reserved_6 : 1;
        std::uint32_t reserved_7 : 1;
        std::uint32_t never_share_resources : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Sound {
        EndianType<SoundFlags> flags;
        EndianType<SoundClass> sound_class;
        EndianType<SoundSampleRate> sample_rate;
        EndianType<float> minimum_distance;
        EndianType<float> maximum_distance;
        EndianType<Fraction> skip_fraction;
        Bounds<EndianType<float>> random_pitch_bounds;
        EndianType<Angle> inner_cone_angle;
        EndianType<Angle> outer_cone_angle;
        EndianType<Fraction> outer_cone_gain;
        EndianType<float> random_gain_modifier;
        EndianType<float> maximum_bend_per_second;
        PAD(0xC);
        EndianType<float> zero_skip_fraction_modifier;
        EndianType<float> zero_gain_modifier;
        EndianType<float> zero_pitch_modifier;
        PAD(0xC);
        EndianType<float> one_skip_fraction_modifier;
        EndianType<float> one_gain_modifier;
        EndianType<float> one_pitch_modifier;
        PAD(0xC);
        EndianType<SoundEncoding> encoding;
        EndianType<SoundCompression> compression;
        TagDependency<EndianType> promotion_sound; // sound
        EndianType<std::int16_t> promotion_count;
        PAD(0x2);
        EndianType<std::uint32_t> unknown_int;
        PAD(0x8);
        LittleEndian<std::uint32_t> unknown_ffffffff_0;
        LittleEndian<std::uint32_t> unknown_ffffffff_1;
        TagReflexive<EndianType, SoundPitchRange> pitch_ranges;

        ENDIAN_TEMPLATE(NewType) operator Sound<NewType>() const noexcept {
            Sound<NewType> copy = {};
            COPY_THIS(flags);
            COPY_THIS(sound_class);
            COPY_THIS(sample_rate);
            COPY_THIS(minimum_distance);
            COPY_THIS(maximum_distance);
            COPY_THIS(skip_fraction);
            COPY_THIS(random_pitch_bounds);
            COPY_THIS(inner_cone_angle);
            COPY_THIS(outer_cone_angle);
            COPY_THIS(outer_cone_gain);
            COPY_THIS(random_gain_modifier);
            COPY_THIS(maximum_bend_per_second);
            COPY_THIS(zero_skip_fraction_modifier);
            COPY_THIS(zero_gain_modifier);
            COPY_THIS(zero_pitch_modifier);
            COPY_THIS(one_skip_fraction_modifier);
            COPY_THIS(one_gain_modifier);
            COPY_THIS(one_pitch_modifier);
            COPY_THIS(encoding);
            COPY_THIS(compression);
            COPY_THIS(promotion_sound);
            COPY_THIS(promotion_count);
            COPY_THIS(unknown_int);
            COPY_THIS(unknown_ffffffff_0);
            COPY_THIS(unknown_ffffffff_1);
            COPY_THIS(pitch_ranges);
            return copy;
        }
    };
    static_assert(sizeof(Sound<BigEndian>) == 0xA4);

    void compile_sound_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
