// SPDX-License-Identifier: GPL-3.0-only

#include <invader/sound/sound_encoder.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/error.hpp>
#include <vorbis/vorbisenc.h>
#include <memory>
#include <cstdint>
#include <samplerate.h>

extern "C" {
#include "adpcm_xq/adpcm-lib.h"
}

namespace Invader::SoundEncoder {
    std::int32_t read_sample(const std::byte *pcm, std::size_t bits_per_sample) noexcept {
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::int32_t sample_value = 0;

        // Get the sample value
        for(std::size_t b = 0; b < bytes_per_sample; b++) {
            std::int32_t significance = 8 * b;
            if(b + 1 == bytes_per_sample) {
                sample_value |= static_cast<std::int64_t>(static_cast<std::int8_t>(pcm[b])) << significance;
            }
            else {
                sample_value |= static_cast<std::int64_t>(static_cast<std::uint8_t>(pcm[b])) << significance;
            }
        }

        return sample_value;
    }

    std::vector<std::byte> convert_to_16_bit_pcm_big_endian(const std::vector<std::byte> &pcm, std::size_t bits_per_sample) {
        // Convert to 16 bits per sample if needed
        std::vector<std::byte> pcm_to_use;
        if(bits_per_sample != 16) {
            pcm_to_use = convert_int_to_int(pcm, bits_per_sample, 16);
        }
        else {
            pcm_to_use = pcm;
        }

        // Swap endianness
        std::uint16_t *samples = reinterpret_cast<std::uint16_t *>(pcm_to_use.data());
        std::size_t sample_count = pcm_to_use.size() / sizeof(*samples);
        for(std::size_t i = 0; i < sample_count; i++) {
            auto sample = samples[i];
            auto *sample_bytes = reinterpret_cast<std::byte *>(samples + i);
            sample_bytes[1] = static_cast<std::byte>(sample & 0xFF);
            sample_bytes[0] = static_cast<std::byte>((sample >> 8) & 0xFF);
        }

        // Done!
        return pcm_to_use;
    }

    std::vector<std::byte> convert_int_to_int(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::size_t new_bits_per_sample) {
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::size_t new_bytes_per_sample = new_bits_per_sample / 8;
        std::size_t sample_count = pcm.size() / bytes_per_sample;
        std::vector<std::byte> samples(sample_count * new_bytes_per_sample);

        // Calculate what we divide by
        std::int64_t divide_by = 1 << bits_per_sample;

        // Calculate what we multiply by
        std::int64_t multiply_by = 1 << new_bits_per_sample;

        auto *pcm_data = pcm.data();
        auto *output_pcm_data = samples.data();

        for(std::size_t i = 0; i < sample_count; i++) {
            // Get the new sample value
            std::int64_t sample = read_sample(pcm_data, bits_per_sample);
            std::int64_t new_sample = sample * multiply_by / divide_by;
            write_sample(static_cast<std::int32_t>(new_sample), output_pcm_data, new_bits_per_sample);

            // Add it
            pcm_data += bytes_per_sample;
            output_pcm_data += new_bytes_per_sample;
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
            std::int64_t sample = read_sample(pcm_data, bits_per_sample);
            samples.emplace_back(sample / divide_by_arr[sample < 0]);
            pcm_data += bytes_per_sample;
        }

        return samples;
    }

    std::vector<std::byte> convert_float_to_int(const std::vector<float> &pcm, std::size_t new_bits_per_sample) {
        std::size_t sample_count = pcm.size();
        std::size_t bytes_per_sample = new_bits_per_sample / 8;
        std::vector<std::byte> samples(sample_count * bytes_per_sample);
        auto *sample_data = samples.data();

        // Calculate what we multiply by
        std::int64_t multiply_by = (1 << new_bits_per_sample) / 2.0;
        std::int64_t multiply_by_minus_one = multiply_by - 1;
        std::int64_t multiply_by_arr[2] = { multiply_by_minus_one, multiply_by };

        for(std::size_t i = 0; i < sample_count; i++) {
            std::int64_t sample = pcm[i] * multiply_by_arr[pcm[i] < 0];

            // Clamp
            if(sample >= multiply_by_minus_one) {
                sample = multiply_by_minus_one;
            }
            else if(sample <= -multiply_by) {
                sample = -multiply_by;
            }

            write_sample(static_cast<std::int32_t>(sample), sample_data, new_bits_per_sample);
            sample_data += bytes_per_sample;
        }

        return samples;
    }

    void write_sample(std::int32_t sample, std::byte *pcm, std::size_t bits_per_sample) noexcept {
        std::size_t bytes_per_sample = bits_per_sample / 8;
        for(std::size_t b = 0; b < bytes_per_sample; b++) {
            pcm[b] = static_cast<std::byte>((sample >> b * 8) & 0xFF);
        }
    }
}
