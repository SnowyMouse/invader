// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SOUND__ENCODER_HPP
#define INVADER__SOUND__ENCODER_HPP

#include <cstddef>
#include <vector>
#include <cstdint>

namespace Invader::SoundEncoder {
    /**
     * Encode the PCM data to Ogg Vorbis
     * @param pcm             PCM data
     * @param bits_per_sample bits per sample of the PCM data
     * @param channel_count   channel count
     * @param sample_rate     sample rate
     * @param vorbis_quality  vorbis quality
     */
    std::vector<std::byte> encode_to_ogg_vorbis(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate, float vorbis_quality);

    /**
     * Encode the PCM data to big endian PCM
     * @param pcm             PCM data
     * @param bits_per_sample bits per sample of the PCM data
     */
    std::vector<std::byte> convert_to_16_bit_pcm_big_endian(const std::vector<std::byte> &pcm, std::size_t bits_per_sample);

    /**
     * Encode from one PCM size to another
     * @param pcm                 PCM data
     * @param bits_per_sample     bits per sample
     * @param new_bits_per_sample new bits per sample
     */
    std::vector<std::byte> convert_int_to_int(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::size_t new_bits_per_sample);

    /**
     * Encode from one PCM size to another
     * @param pcm             PCM data
     * @param bits_per_sample bits per sample
     */
    std::vector<float> convert_int_to_float(const std::vector<std::byte> &pcm, std::size_t bits_per_sample);

    /**
     * Encode from one PCM size to another
     * @param pcm                 PCM data
     * @param new_bits_per_sample new bits per sample
     */
    std::vector<std::byte> convert_float_to_int(const std::vector<float> &pcm, std::size_t new_bits_per_sample);
}

#endif
