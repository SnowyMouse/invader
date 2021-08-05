// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include <invader/tag/parser/definition/sound.hpp>
#include <invader/tag/parser/definition/sound_looping.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/map/tag.hpp>

namespace Invader::Parser {
    void SoundPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        auto permutation_index = offset / sizeof(C<LittleEndian>);
        
        // Flip the endianness of the 16-bit PCM stream
        if(this->format == SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            std::size_t size = this->samples.size() / 2;
            if(size * 2 != this->samples.size()) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound permutation #%zu has an invalid size.", permutation_index);
                throw InvalidTagDataException();
            }

            auto *samples = reinterpret_cast<LittleEndian<std::uint16_t> *>(this->samples.data());
            auto *samples_big = reinterpret_cast<BigEndian<std::uint16_t> *>(this->samples.data());
            for(std::size_t i = 0; i < size; i++) {
                samples_big[i] = samples[i];
            }
        }

        // Warn about this
        bool buffer_size_required = (this->format == SoundFormat::SOUND_FORMAT_16_BIT_PCM || this->format == SoundFormat::SOUND_FORMAT_OGG_VORBIS);
        if(buffer_size_required) {
            bool stock_halo_wont_play_it = false;
            if(this->buffer_size == 0) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Sound permutation #%zu has 0 decompression buffer.", permutation_index);
                stock_halo_wont_play_it = true;
            }
            // Make sure the value is set
            else if(this->format == SoundFormat::SOUND_FORMAT_16_BIT_PCM && this->buffer_size != this->samples.size()) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Sound permutation #%zu has an incorrect decompression buffer size.", permutation_index);
                stock_halo_wont_play_it = true;
            }
            if(stock_halo_wont_play_it) {
                eprintf_warn("Stock Halo may not play it.");
            }
        }
        else {
            this->buffer_size = 0;
        }
        
        // Warn based on format
        auto engine_target = workload.get_build_parameters()->details.build_cache_file_engine;
        
        switch(this->format) {
            case SoundFormat::SOUND_FORMAT_OGG_VORBIS:
                if(engine_target == CacheFileEngine::CACHE_FILE_XBOX) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Sound permutation #%zu uses Ogg Vorbis which does not exist on the target engine", permutation_index);
                }
                break;
            case SoundFormat::SOUND_FORMAT_IMA_ADPCM:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Sound permutation #%zu uses IMA ADPCM which does not exist on the target engine", permutation_index);
                break;
            case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                if(engine_target != CacheFileEngine::CACHE_FILE_NATIVE && engine_target != CacheFileEngine::CACHE_FILE_MCC_CEA) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Sound permutation #%zu uses 16-bit PCM will not play on the original target engine", permutation_index);
                }
                break;
            case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
                // Xbox ADPCM works on everything
                break;
            default:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound permutation #%zu has an invalid sound format set", permutation_index);
                throw InvalidTagDataException();
        }

        // Add the two lone IDs and set the sample size
        auto *data = workload.structs[struct_index].data.data();
        auto &this_struct = *reinterpret_cast<C<LittleEndian> *>(data + offset);
        this_struct.samples.size = static_cast<std::uint32_t>(this->samples.size());

        // Who am I??
        auto &new_id_1 = workload.structs[struct_index].dependencies.emplace_back();
        new_id_1.tag_index = tag_index;
        new_id_1.offset = reinterpret_cast<std::byte *>(&this_struct.tag_id_0) - data;
        new_id_1.tag_id_only = true;

        // 24601!!!!!!!
        auto &new_id_2 = workload.structs[struct_index].dependencies.emplace_back();
        new_id_2.tag_index = tag_index;
        new_id_2.offset = reinterpret_cast<std::byte *>(&this_struct.tag_id_1) - data;
        new_id_2.tag_id_only = true;

        // Add samples
        auto &r = workload.raw_data.emplace_back(this->samples);
        workload.tags[tag_index].asset_data.emplace_back(&r - workload.raw_data.data());
        this->samples_pointer = 0xFFFFFFFF;
    }

    void SoundPitchRange::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t struct_offset) {
        this->unknown_ffffffff_0 = 0xFFFFFFFF;
        this->unknown_ffffffff_1 = 0xFFFFFFFF;
        auto actual_natural_pitch = this->natural_pitch <= 0.0F ? 1.0F : this->natural_pitch;
        this->playback_rate = 1.0f / actual_natural_pitch;
        
        // Make sure our bend bounds are valid for the natural pitch
        auto old_bend_bounds = this->bend_bounds;
        
        // Set the new bend bounds, ensuring natural pitch falls within it
        this->bend_bounds.from = std::min(actual_natural_pitch, old_bend_bounds.from);
        this->bend_bounds.to = std::max(actual_natural_pitch, old_bend_bounds.to);
        
        // If our bend bounds was changed, but they weren't zero, then that means bullshit happened
        if((old_bend_bounds.from != 0.0F || old_bend_bounds.to != 0.0F) && ((this->bend_bounds.from != old_bend_bounds.from) || (this->bend_bounds.to != old_bend_bounds.to))) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Natural pitch (%f) in pitch range #%zu falls outside of bend bounds (%f - %f) so the bounds were changed to %f - %f", actual_natural_pitch, struct_offset / sizeof(C<LittleEndian>), old_bend_bounds.from, old_bend_bounds.to, this->bend_bounds.from, this->bend_bounds.to);
        }

        // Make sure all of the permutations are valid
        std::size_t permutation_count = this->permutations.size();
        std::size_t actual_permutation_count = this->actual_permutation_count;
        if(actual_permutation_count > permutation_count) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Actual permutation count in pitch range #%zu exceeds the number of permutations (%zu >= %zu)", struct_offset / sizeof(C<LittleEndian>), actual_permutation_count, permutation_count);
            throw InvalidTagDataException();
        }

        // Now check errors and unused permutations
        std::vector<bool> referenced(permutation_count, false);
        std::fill(referenced.begin(), referenced.begin() + actual_permutation_count, true);
        for(std::size_t i = 0; i < permutation_count; i++) {
            auto &permutation = this->permutations[i];
            std::size_t next_permutation = permutation.next_permutation_index;
            if(next_permutation == NULL_INDEX) {
                continue;
            }
            else if(next_permutation >= permutation_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Permutation #%zu in pitch range #%zu has an invalid next index (%zu >= %zu)", i, struct_offset / sizeof(C<LittleEndian>), next_permutation, permutation_count);
                throw InvalidTagDataException();
            }
            else {
                referenced[next_permutation] = true;
            }
        }

        // List unused permutations
        for(std::size_t i = 0; i < permutation_count; i++) {
            if(!referenced[i]) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Permutation #%zu in pitch range #%zu is unused", i, struct_offset / sizeof(C<LittleEndian>));
            }
        }

        // Look for endless loops
        for(std::size_t a = 0; a < actual_permutation_count; a++) {
            // Reset referenced to all false
            std::fill(referenced.begin(), referenced.begin() + permutation_count, false);

            // Start at where we are
            std::size_t permutation_index = a;

            // Keep going until we hit a null index or we loop forever
            while(permutation_index != NULL_INDEX) {
                if(referenced[permutation_index]) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Actual permutation #%zu (%s) in pitch range #%zu will never end", a, this->permutations[a].name.string, struct_offset / sizeof(C<LittleEndian>));
                    throw InvalidTagDataException();
                }
                referenced[permutation_index] = true;
                permutation_index = this->permutations[permutation_index].next_permutation_index;
            }
        }
    }

    template <typename T> static void sound_pre_compile(T *sound, BuildWorkload &workload, std::size_t tag_index) {
        sound->maximum_bend_per_second = std::pow(sound->maximum_bend_per_second, 1.0f / TICK_RATE);
        sound->unknown_ffffffff_0 = 0xFFFFFFFF;
        sound->unknown_ffffffff_1 = 0xFFFFFFFF;

        std::size_t errors = 0;
        for(auto &pr : sound->pitch_ranges) {
            for(auto &pe : pr.permutations) {
                if(pe.format != sound->format) {
                    errors++;
                }
            }
        }
        if(errors) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound format does not match %zu permutation%s.", errors, errors == 1 ? "" : "s");
            throw InvalidTagDataException();
        }

        if(sound->channel_count == SoundChannelCount::SOUND_CHANNEL_COUNT_MONO && sound->sample_rate == SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ && workload.get_build_parameters()->details.build_cache_file_engine != CacheFileEngine::CACHE_FILE_NATIVE) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Sound is 44.1 kHz AND mono. The target engine will not play this.");
        }

        // If we didn't split long sounds into permutations, go through each permutation and Jason Jones it
        if(!(sound->flags & SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS)) {
            for(auto &pr : sound->pitch_ranges) {
                pr.actual_permutation_count = static_cast<std::uint16_t>(pr.permutations.size());
                for(auto &p : pr.permutations) {
                    p.next_permutation_index = NULL_INDEX;
                }
            }
        }
        
        if(sound->one_skip_fraction_modifier == 0.0f && sound->zero_skip_fraction_modifier == 0.0f) {
            sound->one_skip_fraction_modifier = 1.0f;
            sound->zero_skip_fraction_modifier = 1.0f;
        }

        if(sound->one_gain_modifier == 0.0f && sound->zero_gain_modifier == 0.0f) {
            sound->one_gain_modifier = 1.0f;
        
            // Set default zero gain modifier based on class
            switch(sound->sound_class) {
                case SoundClass::SOUND_CLASS_OBJECT_IMPACTS:
                case SoundClass::SOUND_CLASS_PARTICLE_IMPACTS:
                case SoundClass::SOUND_CLASS_SLOW_PARTICLE_IMPACTS:
                case SoundClass::SOUND_CLASS_UNIT_DIALOG:
                case SoundClass::SOUND_CLASS_MUSIC:
                case SoundClass::SOUND_CLASS_AMBIENT_NATURE:
                case SoundClass::SOUND_CLASS_AMBIENT_MACHINERY:
                case SoundClass::SOUND_CLASS_AMBIENT_COMPUTERS:
                case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_PLAYER:
                case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_OTHER:
                case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_FORCE_UNSPATIALIZED:
                case SoundClass::SOUND_CLASS_SCRIPTED_EFFECT:
                    sound->zero_gain_modifier = 0.0F;
                    break;
                default:
                    sound->zero_gain_modifier = 1.0F;
                    break;
            }
        }

        if(sound->zero_pitch_modifier == 0.0f && sound->one_pitch_modifier == 0.0f) {
            sound->one_pitch_modifier = 1.0f;
            sound->zero_pitch_modifier = 1.0f;
        }
    }

    void Sound::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        sound_pre_compile(this, workload, tag_index);
        
        // Warn if we're using bullshit distances
        if(this->minimum_distance > this->maximum_distance) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Minimum distance is greater than maximum distance (%f > %f)", this->maximum_distance, this->minimum_distance);
        }
        
        // Find the song length
        if(this->zero_pitch_modifier == 1.0F && this->one_pitch_modifier == 1.0F) {
            double seconds = 0.0F;
            for(auto &pr : this->pitch_ranges) {
                for(auto &p : pr.permutations) {
                    std::size_t sample_count = 0;
                    switch(p.format) {
                        case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
                            sample_count = (p.samples.size() / 36 * 130) / sizeof(std::uint16_t);
                            break;
                        default:
                            sample_count = p.buffer_size / 2;
                            break;
                    }
                    
                    double potential_seconds = sample_count / (this->channel_count == SoundChannelCount::SOUND_CHANNEL_COUNT_MONO ? 1.0 : 2.0) / (this->sample_rate == SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ ? 44100.0 : 22050.0) * pr.natural_pitch;
                    
                    if(potential_seconds > seconds) {
                        seconds = potential_seconds;
                    }
                }
            }
            this->longest_permutation_length = seconds * 1100;
        }
    }

    void Sound::post_cache_parse(const Invader::Tag &tag, std::optional<Pointer>) {
        this->maximum_bend_per_second = std::pow(this->maximum_bend_per_second, TICK_RATE);
        if(tag.is_indexed()) {
            auto &tag_data = *(reinterpret_cast<const C<LittleEndian> *>(&tag.get_struct_at_pointer<SoundPitchRange::C>(static_cast<Pointer>(0), 0)) - 1);
            this->format = tag_data.format;
            this->channel_count = tag_data.channel_count;
            this->sample_rate = tag_data.sample_rate;
        }
    }

    void SoundPermutation::post_cache_deformat() {
        if(this->format == SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            auto *start = reinterpret_cast<LittleEndian<std::uint16_t> *>(this->samples.data());
            auto *end = start + this->samples.size() / sizeof(*start);

            while(start < end) {
                *reinterpret_cast<BigEndian<std::uint16_t> *>(start) = *start;
                start++;
            }
        }
    }

    void SoundLooping::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        if(this->zero_detail_sound_period == 0.0f && this->one_detail_sound_period == 0.0f) {
            this->zero_detail_sound_period = 1.0f;
            this->one_detail_sound_period = 1.0f;
        }

        this->unknown_int = 0xFFFFFFFF; // this is probably a pointer, but we should check to make sure it isn't something important

        std::fill(this->one_detail_unknown_floats, this->one_detail_unknown_floats + 2, 1.0F);
        std::fill(this->zero_detail_unknown_floats, this->zero_detail_unknown_floats + 2, 1.0F);
    }

    void SoundLooping::post_compile(BuildWorkload &workload, std::size_t, std::size_t struct_index, std::size_t struct_offset) {
        auto &maximum_distance = reinterpret_cast<SoundLooping::C<LittleEndian> *>(workload.structs[struct_index].data.data() + struct_offset)->maximum_distance;
        
        auto get_max_distance_of_sound_tag = [&workload](const ParsedTagDependency &tag) {
            if(tag.tag_id.is_null()) {
                return 0.0F;
            }
            
            auto &sound_tag_data = *reinterpret_cast<const Sound::C<LittleEndian> *>(workload.structs[*workload.tags[tag.tag_id.index].base_struct].data.data());
            float max = sound_tag_data.maximum_distance.read();
            if(max == 0.0F) {
                switch(sound_tag_data.sound_class) {
                    case SoundClass::SOUND_CLASS_OBJECT_IMPACTS:
                    case SoundClass::SOUND_CLASS_PARTICLE_IMPACTS:
                    case SoundClass::SOUND_CLASS_SLOW_PARTICLE_IMPACTS:
                    case SoundClass::SOUND_CLASS_DEVICE_COMPUTERS:
                    case SoundClass::SOUND_CLASS_AMBIENT_COMPUTERS:
                    case SoundClass::SOUND_CLASS_FIRST_PERSON_DAMAGE:
                        return 3.0F;

                    case SoundClass::SOUND_CLASS_DEVICE_DOOR:
                    case SoundClass::SOUND_CLASS_DEVICE_FORCE_FIELD:
                    case SoundClass::SOUND_CLASS_DEVICE_MACHINERY:
                    case SoundClass::SOUND_CLASS_DEVICE_NATURE:
                    case SoundClass::SOUND_CLASS_MUSIC:
                    case SoundClass::SOUND_CLASS_AMBIENT_NATURE:
                    case SoundClass::SOUND_CLASS_AMBIENT_MACHINERY:
                    case SoundClass::SOUND_CLASS_SCRIPTED_EFFECT:
                        return 5.0F;

                    case SoundClass::SOUND_CLASS_PROJECTILE_IMPACT:
                    case SoundClass::SOUND_CLASS_VEHICLE_COLLISION:
                    case SoundClass::SOUND_CLASS_VEHICLE_ENGINE:
                        return 8.0F;

                    case SoundClass::SOUND_CLASS_WEAPON_READY:
                    case SoundClass::SOUND_CLASS_WEAPON_RELOAD:
                    case SoundClass::SOUND_CLASS_WEAPON_EMPTY:
                    case SoundClass::SOUND_CLASS_WEAPON_CHARGE:
                    case SoundClass::SOUND_CLASS_WEAPON_OVERHEAT:
                    case SoundClass::SOUND_CLASS_WEAPON_IDLE:
                        return 9.0F;

                    case SoundClass::SOUND_CLASS_UNIT_FOOTSTEPS:
                        return 10.0F;

                    case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_PLAYER:
                    case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_OTHER:
                    case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_FORCE_UNSPATIALIZED:
                    case SoundClass::SOUND_CLASS_GAME_EVENT:
                    case SoundClass::SOUND_CLASS_UNIT_DIALOG:
                        return 20.0F;

                    case SoundClass::SOUND_CLASS_WEAPON_FIRE:
                        return 70.0F;

                    case SoundClass::SOUND_CLASS_PROJECTILE_DETONATION:
                        return 120.0F;
                        
                    default:
                        break;
                }
            }
            
            return max;
        };
        
        // Get the maximum distances of all referenced sound tags
        for(auto &i : this->tracks) {
            maximum_distance = std::max(std::max(std::initializer_list<float> {
                get_max_distance_of_sound_tag(i.loop),
                get_max_distance_of_sound_tag(i.alternate_end),
                get_max_distance_of_sound_tag(i.alternate_loop),
                get_max_distance_of_sound_tag(i.start),
                get_max_distance_of_sound_tag(i.end)
            }), maximum_distance.read());
        }
        for(auto &i : this->detail_sounds) {
            maximum_distance = std::max(get_max_distance_of_sound_tag(i.sound), maximum_distance.read());
        }
    }
}
