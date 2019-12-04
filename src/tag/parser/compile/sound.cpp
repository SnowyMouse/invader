// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void SoundPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        // Flip the endianness of the 16-bit PCM stream
        if(this->format == HEK::SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            std::size_t size = this->samples.size() / 2;
            if(size * 2 != this->samples.size()) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Sound permutation #%zu has an invalid size", offset / sizeof(struct_little));
                throw InvalidTagDataException();
            }

            auto *samples = reinterpret_cast<HEK::LittleEndian<std::uint16_t> *>(this->samples.data());
            auto *samples_big = reinterpret_cast<HEK::BigEndian<std::uint16_t> *>(this->samples.data());
            for(std::size_t i = 0; i < size; i++) {
                samples_big[i] = samples[i];
            }
        }

        // Add the two lone IDs
        auto *data = workload.structs[struct_index].data.data();
        auto &this_struct = *reinterpret_cast<struct_little *>(data + offset);

        auto &new_id_1 = workload.structs[struct_index].dependencies.emplace_back();
        new_id_1.tag_index = tag_index;
        new_id_1.offset = reinterpret_cast<std::byte *>(&this_struct.tag_id_0) - data;
        new_id_1.tag_id_only = true;

        auto &new_id_2 = workload.structs[struct_index].dependencies.emplace_back();
        new_id_2.tag_index = tag_index;
        new_id_1.offset = reinterpret_cast<std::byte *>(&this_struct.tag_id_1) - data;
        new_id_2.tag_id_only = true;

        // Add samples
        auto &r = workload.raw_data.emplace_back(this->samples);
        workload.tags[tag_index].asset_data.emplace_back(&r - workload.raw_data.data());
    }

    void SoundPitchRange::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->unknown_ffffffff_0 = 0xFFFFFFFF;
        this->unknown_ffffffff_1 = 0xFFFFFFFF;
        this->playback_rate = 1.0f / (this->natural_pitch == 0.0F ? 1.0F : this->natural_pitch);
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
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "Sound format does not match %zu permutation%s", errors, errors == 1 ? "" : "s");
        }
    }
}
