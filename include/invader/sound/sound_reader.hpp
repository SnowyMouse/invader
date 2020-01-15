// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SOUND__READER_HPP
#define INVADER__SOUND__READER_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace Invader::SoundReader {
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

        /** Ignore this */
        void *internal;
    };

    /**
     * Get the sound from a WAV file
     * @param  path path to the file
     * @return      sound
     */
    Sound sound_from_wav_file(const char *path);

    /**
     * Get the sound from WAV data
     * @param  data        pointer to data
     * @param  data_length data size
     * @return             sound
     */
    Sound sound_from_wav(const std::byte *data, std::size_t data_length);

    /**
     * Get the sound from a Ogg file
     * @param  path path to the file
     * @return      sound
     */
    Sound sound_from_ogg_file(const char *path);

    /**
     * Get the sound from Ogg data
     * @param  data        pointer to data
     * @param  data_length data size
     * @return             sound
     */
    Sound sound_from_ogg(const std::byte *data, std::size_t data_length);

    /**
     * Get the sound from a FLAC file
     * @param  path path to the file
     * @return      sound
     */
    Sound sound_from_flac_file(const char *path);

    /**
     * Get the sound from FLAC data
     * @param  data        pointer to data
     * @param  data_length data size
     * @return             sound
     */
    Sound sound_from_flac(const std::byte *data, std::size_t data_length);
}

#endif
