// SPDX-License-Identifier: GPL-3.0-only

#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/sound/sound_reader.hpp>

namespace Invader::SoundReader {
    Sound sound_from_16_bit_pcm_big_endian(const std::byte *data, std::size_t data_length, std::size_t channel_count, std::size_t sample_rate) {
        Sound result = {};

        // Set parameters
        result.input_bits_per_sample = 16;
        result.bits_per_sample = 16;
        result.channel_count = channel_count;
        result.input_channel_count = result.channel_count;
        result.sample_rate = sample_rate;
        result.input_sample_rate = result.sample_rate;

        if(data_length % (sizeof(std::uint16_t) * channel_count) != 0) {
            eprintf_error("Data length is not divisible by 2 x channel_count");
            throw InvalidInputSoundException();
        }

        // Allocate
        result.pcm = std::vector<std::byte>(data_length);
        std::size_t sample_count = data_length / sizeof(std::uint16_t);

        auto *output_data = result.pcm.data();
        const auto *input_data = data;

        // Swap the endianness to little endian
        for(std::size_t i = 0; i < sample_count; i++) {
            output_data[0] = input_data[1];
            output_data[1] = input_data[0];
            input_data += sizeof(std::uint16_t);
            output_data += sizeof(std::uint16_t);
        }

        // Return the result
        return result;
    }
}
