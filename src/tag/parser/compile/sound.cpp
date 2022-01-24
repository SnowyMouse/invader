// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/sound/sound_reader.hpp>

namespace Invader::Parser {
    static std::pair<float, float> default_min_max_distance_sounds(HEK::SoundClass sclass) {
        switch(sclass) {
            case HEK::SoundClass::SOUND_CLASS_DEVICE_MACHINERY:
            case HEK::SoundClass::SOUND_CLASS_DEVICE_FORCE_FIELD:
            case HEK::SoundClass::SOUND_CLASS_AMBIENT_MACHINERY:
            case HEK::SoundClass::SOUND_CLASS_AMBIENT_NATURE:
            case HEK::SoundClass::SOUND_CLASS_DEVICE_DOOR:
            case HEK::SoundClass::SOUND_CLASS_MUSIC:
            case HEK::SoundClass::SOUND_CLASS_DEVICE_NATURE:
                return {0.9F, 5.0F};
                
            case HEK::SoundClass::SOUND_CLASS_WEAPON_EMPTY:
            case HEK::SoundClass::SOUND_CLASS_WEAPON_IDLE:
            case HEK::SoundClass::SOUND_CLASS_WEAPON_READY:
            case HEK::SoundClass::SOUND_CLASS_WEAPON_RELOAD:
            case HEK::SoundClass::SOUND_CLASS_WEAPON_CHARGE:
            case HEK::SoundClass::SOUND_CLASS_WEAPON_OVERHEAT:
                return {1.0F, 9.0F};
                
            case HEK::SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_OTHER:
            case HEK::SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_PLAYER:
            case HEK::SoundClass::SOUND_CLASS_GAME_EVENT:
            case HEK::SoundClass::SOUND_CLASS_UNIT_DIALOG:
            case HEK::SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_FORCE_UNSPATIALIZED:
                return {3.0F, 20.0F};
                
            case HEK::SoundClass::SOUND_CLASS_FIRST_PERSON_DAMAGE:
            case HEK::SoundClass::SOUND_CLASS_OBJECT_IMPACTS:
            case HEK::SoundClass::SOUND_CLASS_AMBIENT_COMPUTERS:
            case HEK::SoundClass::SOUND_CLASS_PARTICLE_IMPACTS:
            case HEK::SoundClass::SOUND_CLASS_DEVICE_COMPUTERS:
            case HEK::SoundClass::SOUND_CLASS_SLOW_PARTICLE_IMPACTS:
                return {0.5F, 3.0F};
                
            case HEK::SoundClass::SOUND_CLASS_VEHICLE_ENGINE:
            case HEK::SoundClass::SOUND_CLASS_PROJECTILE_IMPACT:
            case HEK::SoundClass::SOUND_CLASS_VEHICLE_COLLISION:
                return {1.4F, 8.0F};
                
            case HEK::SoundClass::SOUND_CLASS_WEAPON_FIRE:
                return {4.0F, 70.0F};
            case HEK::SoundClass::SOUND_CLASS_SCRIPTED_EFFECT:
                return {2.0F, 5.0F};
            case HEK::SoundClass::SOUND_CLASS_PROJECTILE_DETONATION:
                return {8.0F, 120.0F};
            case HEK::SoundClass::SOUND_CLASS_UNIT_FOOTSTEPS:
                return {0.9F, 10.0F};
                
            default:
                return {};
        }
    }
    
    
    void SoundPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        auto permutation_index = offset / sizeof(struct_little);

        // Flip the endianness of the 16-bit PCM stream
        if(this->format == HEK::SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            std::size_t size = this->samples.size() / 2;
            if(size * 2 != this->samples.size()) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound permutation #%zu has an invalid size.", permutation_index);
                throw InvalidTagDataException();
            }

            auto *samples = reinterpret_cast<HEK::LittleEndian<std::uint16_t> *>(this->samples.data());
            auto *samples_big = reinterpret_cast<HEK::BigEndian<std::uint16_t> *>(this->samples.data());
            for(std::size_t i = 0; i < size; i++) {
                samples_big[i] = samples[i];
            }
        }

        // Check sound buffer size is what it should be
        bool incorrect_buffer = false;

        if(this->format == HEK::SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            if(this->buffer_size != this->samples.size()) {
                incorrect_buffer = true;
            }
        }
        else if(this->format ==  HEK::SoundFormat::SOUND_FORMAT_OGG_VORBIS) {
            auto decoded = Invader::SoundReader::sound_from_ogg(this->samples.data(), this->samples.size());
            auto decoded_size_16_bit = decoded.pcm.size() / (decoded.bits_per_sample / 8) * 2;
            if(this->buffer_size != decoded_size_16_bit) {
                incorrect_buffer = true;
            }
        }
        else {
            this->buffer_size = 0;
        }

        if(incorrect_buffer) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound permutation #%zu has an incorrect sound buffer size.", permutation_index);
            throw InvalidTagDataException();
        }

        // Error based on format
        auto engine_target = workload.get_build_parameters()->details.build_cache_file_engine;

        switch(this->format) {
            case HEK::SoundFormat::SOUND_FORMAT_OGG_VORBIS:
                if(engine_target == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Sound permutation #%zu uses Ogg Vorbis which does not exist on the target engine.", permutation_index);
                }
                break;
            case HEK::SoundFormat::SOUND_FORMAT_IMA_ADPCM:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Sound permutation #%zu uses IMA ADPCM which does not exist on the target engine.", permutation_index);
                break;
            case HEK::SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                if(engine_target != HEK::CacheFileEngine::CACHE_FILE_NATIVE && engine_target != HEK::CacheFileEngine::CACHE_FILE_MCC_CEA) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Sound permutation #%zu uses 16-bit PCM. The target engine will not play this without a mod.", permutation_index);
                }
                break;
            case HEK::SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
                // Xbox ADPCM works on everything
                break;
            default:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound permutation #%zu has an invalid sound format set.", permutation_index);
                throw InvalidTagDataException();
        }

        // Add the two lone IDs and set the sample size
        auto *data = workload.structs[struct_index].data.data();
        auto &this_struct = *reinterpret_cast<struct_little *>(data + offset);
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
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Natural pitch (%f) in pitch range #%zu falls outside of bend bounds (%f - %f) so the bounds were changed to %f - %f.", actual_natural_pitch, struct_offset / sizeof(struct_little), old_bend_bounds.from, old_bend_bounds.to, this->bend_bounds.from, this->bend_bounds.to);
        }

        // Make sure all of the permutations are valid
        std::size_t permutation_count = this->permutations.size();
        std::size_t actual_permutation_count = this->actual_permutation_count;
        if(actual_permutation_count > permutation_count) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Actual permutation count in pitch range #%zu exceeds the number of permutations (%zu >= %zu)", struct_offset / sizeof(struct_little), actual_permutation_count, permutation_count);
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
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Permutation #%zu in pitch range #%zu has an invalid next index (%zu >= %zu)", i, struct_offset / sizeof(struct_little), next_permutation, permutation_count);
                throw InvalidTagDataException();
            }
            else {
                referenced[next_permutation] = true;
            }
        }

        // List unused permutations
        for(std::size_t i = 0; i < permutation_count; i++) {
            if(!referenced[i]) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Permutation #%zu in pitch range #%zu is unused.", i, struct_offset / sizeof(struct_little));
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
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Actual permutation #%zu (%s) in pitch range #%zu will never end.", a, this->permutations[a].name.string, struct_offset / sizeof(struct_little));
                    throw InvalidTagDataException();
                }
                referenced[permutation_index] = true;
                permutation_index = this->permutations[permutation_index].next_permutation_index;
            }
        }
    }

    void Sound::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->maximum_bend_per_second = std::pow(this->maximum_bend_per_second, 1.0f / TICK_RATE);
        this->unknown_ffffffff_0 = 0xFFFFFFFF;
        this->unknown_ffffffff_1 = 0xFFFFFFFF;

        std::size_t errors = 0;
        for(auto &pr : this->pitch_ranges) {
            for(auto &pe : pr.permutations) {
                if(pe.format != this->format) {
                    errors++;
                }
            }
        }
        if(errors) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound format does not match %zu permutation%s.", errors, errors == 1 ? "" : "s");
            throw InvalidTagDataException();
        }

        if(this->channel_count == HEK::SoundChannelCount::SOUND_CHANNEL_COUNT_MONO && this->sample_rate == HEK::SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ && workload.get_build_parameters()->details.build_cache_file_engine != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Sound is 44.1 kHz AND mono. The target engine will not play this.");
        }

        // If we didn't split long thiss into permutations, go through each permutation and Jason Jones it
        if(!(this->flags & HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS)) {
            for(auto &pr : this->pitch_ranges) {
                pr.actual_permutation_count = static_cast<std::uint16_t>(pr.permutations.size());
                for(auto &p : pr.permutations) {
                    p.next_permutation_index = NULL_INDEX;
                }
            }
        }

        if(this->one_skip_fraction_modifier == 0.0f && this->zero_skip_fraction_modifier == 0.0f) {
            this->one_skip_fraction_modifier = 1.0f;
            this->zero_skip_fraction_modifier = 1.0f;
        }

        if(this->one_gain_modifier == 0.0f && this->zero_gain_modifier == 0.0f) {
            this->one_gain_modifier = 1.0f;

            // Set default zero gain modifier based on class
            switch(this->sound_class) {
                case HEK::SoundClass::SOUND_CLASS_OBJECT_IMPACTS:
                case HEK::SoundClass::SOUND_CLASS_PARTICLE_IMPACTS:
                case HEK::SoundClass::SOUND_CLASS_SLOW_PARTICLE_IMPACTS:
                case HEK::SoundClass::SOUND_CLASS_UNIT_DIALOG:
                case HEK::SoundClass::SOUND_CLASS_MUSIC:
                case HEK::SoundClass::SOUND_CLASS_AMBIENT_NATURE:
                case HEK::SoundClass::SOUND_CLASS_AMBIENT_MACHINERY:
                case HEK::SoundClass::SOUND_CLASS_AMBIENT_COMPUTERS:
                case HEK::SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_PLAYER:
                case HEK::SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_OTHER:
                case HEK::SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_FORCE_UNSPATIALIZED:
                case HEK::SoundClass::SOUND_CLASS_SCRIPTED_EFFECT:
                    this->zero_gain_modifier = 0.0F;
                    break;
                default:
                    this->zero_gain_modifier = 1.0F;
                    break;
            }
        }

        if(this->zero_pitch_modifier == 0.0f && this->one_pitch_modifier == 0.0f) {
            this->one_pitch_modifier = 1.0f;
            this->zero_pitch_modifier = 1.0f;
        }
        
        // Set distances
        auto default_distance = default_min_max_distance_sounds(this->sound_class);
        if(this->minimum_distance == 0.0F) {
            this->minimum_distance = default_distance.first;
        }
        if(this->maximum_distance == 0.0F) {
            this->maximum_distance = default_distance.second;
        }

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
                        case HEK::SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
                            sample_count = (p.samples.size() / 36 * 130) / sizeof(std::uint16_t);
                            break;
                        default:
                            sample_count = p.buffer_size / 2;
                            break;
                    }

                    double potential_seconds = sample_count / (this->channel_count == HEK::SoundChannelCount::SOUND_CHANNEL_COUNT_MONO ? 1.0 : 2.0) / (this->sample_rate == HEK::SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ ? 44100.0 : 22050.0) * pr.natural_pitch;

                    if(potential_seconds > seconds) {
                        seconds = potential_seconds;
                    }
                }
            }
            this->longest_permutation_length = seconds * 1100;
        }
    }

    void Sound::post_cache_parse(const Invader::Tag &tag, std::optional<HEK::Pointer>) {
        this->maximum_bend_per_second = std::pow(this->maximum_bend_per_second, TICK_RATE);
        if(tag.is_indexed()) {
            auto &tag_data = *(reinterpret_cast<const struct_little *>(&tag.get_struct_at_pointer<HEK::SoundPitchRange>(static_cast<HEK::Pointer>(0), 0)) - 1);
            this->format = tag_data.format;
            this->channel_count = tag_data.channel_count;
            this->sample_rate = tag_data.sample_rate;
        }
    }

    // Unflip the endianness of a 16-bit PCM stream
    void SoundPermutation::post_cache_deformat() {
        if(this->format == HEK::SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            auto *start = reinterpret_cast<HEK::LittleEndian<std::uint16_t> *>(this->samples.data());
            auto *end = start + this->samples.size() / sizeof(*start);

            while(start < end) {
                *reinterpret_cast<HEK::BigEndian<std::uint16_t> *>(start) = *start;
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
        if(workload.disable_recursion) {
            return;
        }

        auto &maximum_distance = reinterpret_cast<SoundLooping::struct_little *>(workload.structs[struct_index].data.data() + struct_offset)->maximum_distance;

        auto get_max_distance_of_sound_tag = [&workload](const Dependency &tag) -> float {
            if(tag.tag_id.is_null()) {
                return 0.0F;
            }

            auto &sound_tag_data = *reinterpret_cast<const Sound::struct_little *>(workload.structs[*workload.tags[tag.tag_id.index].base_struct].data.data());
            return sound_tag_data.maximum_distance.read();
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
