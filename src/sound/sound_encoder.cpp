// SPDX-License-Identifier: GPL-3.0-only

#include <invader/sound/sound_encoder.hpp>

namespace Invader::SoundEncoder {
    // std::vector<std::byte> convert_to_ogg_vorbis(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate);

    // std::vector<std::byte> convert_to_16_bit_pcm_big_endian(const std::vector<std::byte> &pcm, std::size_t bits_per_sample);

    std::vector<std::byte> convert_int_to_int(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::size_t new_bits_per_sample) {
        std::size_t bytes_per_sample = bits_per_sample;
        std::size_t new_bytes_per_sample = new_bits_per_sample / 8;
        std::size_t sample_count = pcm.size() / bits_per_sample;

        std::vector<std::byte> samples;
        samples.reserve(sample_count * new_bytes_per_sample);

        // Calculate what we divide by
        std::int64_t divide_by = 1 << bits_per_sample;

        // Calculate what we multiply by
        std::int64_t multiply_by = 1 << new_bits_per_sample;

        auto *pcm_data = pcm.data();

        for(std::size_t i = 0; i < sample_count; i++) {
            std::int64_t sample = 0;

            // Get the sample value
            for(std::size_t b = 0; b < bytes_per_sample; b++) {
                std::int64_t significance = 8 * b;
                if(b + 1 == bytes_per_sample) {
                    sample |= static_cast<std::int64_t>(static_cast<std::int8_t>(pcm_data[b])) << significance;
                }
                else {
                    sample |= static_cast<std::int64_t>(static_cast<std::uint8_t>(pcm_data[b])) << significance;
                }
            }

            // Get the new sample value
            std::int64_t new_sample = sample * multiply_by / divide_by;
            for(std::size_t b = 0; b < new_bytes_per_sample; b++) {
                samples.emplace_back(static_cast<std::byte>((new_sample >> b * 8) & 0xFF));
            }

            // Add it
            pcm_data += bytes_per_sample;
        }

        return samples;
    }

    std::vector<float> convert_int_to_float(const std::vector<std::byte> &pcm, std::size_t bits_per_sample) {
        std::vector<float> samples;
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::size_t sample_count = pcm.size() / bytes_per_sample;
        samples.reserve(sample_count);

        // Calculate what we divide by
        float divide_by = (1 << bits_per_sample) / 2.0F;
        float divide_by_minus_one = divide_by - 1;
        float divide_by_arr[2] = { divide_by_minus_one, divide_by };

        auto *pcm_data = pcm.data();

        for(std::size_t i = 0; i < sample_count; i++) {
            std::int64_t sample = 0;

            // Get the sample value
            for(std::size_t b = 0; b < bytes_per_sample; b++) {
                std::int64_t significance = 8 * b;
                if(b + 1 == bytes_per_sample) {
                    sample |= static_cast<std::int64_t>(static_cast<std::int8_t>(pcm_data[b])) << significance;
                }
                else {
                    sample |= static_cast<std::int64_t>(static_cast<std::uint8_t>(pcm_data[b])) << significance;
                }
            }

            // Add it
            samples.emplace_back(sample / divide_by_arr[sample < 0]);
            pcm_data += bytes_per_sample;
        }

        return samples;
    }

    std::vector<std::byte> convert_float_to_int(const std::vector<float> &pcm, std::size_t new_bits_per_sample) {
        std::vector<std::byte> samples;
        std::size_t sample_count = pcm.size();
        std::size_t bytes_per_sample = new_bits_per_sample / 8;
        samples.reserve(sample_count * bytes_per_sample);

        // Calculate what we multiply by
        std::int64_t multiply_by = (1 << new_bits_per_sample) / 2.0;
        std::int64_t multiply_by_minus_one = multiply_by - 1;
        std::int64_t multiply_by_arr[2] = { multiply_by_minus_one, multiply_by };

        for(std::size_t i = 0; i < sample_count; i++) {
            std::int64_t sample = pcm[i] * multiply_by_arr[pcm[i] < 0];
            for(std::size_t b = 0; b < bytes_per_sample; b++) {
                samples.emplace_back(static_cast<std::byte>((sample >> b * 8) & 0xFF));
            }
        }

        return samples;
    }
}
