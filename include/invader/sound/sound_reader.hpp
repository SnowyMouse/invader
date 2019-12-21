// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SOUND__READER_HPP
#define INVADER__SOUND__READER_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace Invader {
    class SoundReader {
    public:
        struct Sound {
            /** Sample rate */
            std::uint32_t sample_rate;

            /** Channel count */
            std::uint16_t channel_count;

            /** Number of bits per sample */
            std::uint32_t bits_per_sample;

            /** Sample rate of input file */
            std::uint32_t input_sample_rate;

            /** Channel count of input file */
            std::uint32_t input_channel_count;

            /** Number of bits per sample of input file */
            std::uint32_t input_bits_per_sample;

            /** PCM data */
            std::vector<std::byte> pcm;

            /** Name */
            std::string name;
        };

        /**
         * Get the sound from a WAV file
         * @param  data        pointer to the data
         * @param  data_length length of the data
         * @return             sound
         */
        static Sound sound_from_wav(const std::byte *data, std::size_t data_length);

    private:
        SoundReader() = default;

    };
}

#endif
