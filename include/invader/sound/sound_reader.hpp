// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SOUND__READER_HPP
#define INVADER__SOUND__READER_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <filesystem>

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
    Sound sound_from_wav_file(const std::filesystem::path &path);

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
    Sound sound_from_ogg_file(const std::filesystem::path &path);

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
    Sound sound_from_flac_file(const std::filesystem::path &path);

    /**
     * Get the sound from FLAC data
     * @param  data        pointer to data
     * @param  data_length data size
     * @return             sound
     */
    Sound sound_from_flac(const std::byte *data, std::size_t data_length);

    /**
     * Get the sound from Xbox ADPCM data
     * @param  data          pointer to data
     * @param  data_length   data size
     * @param  channel_count number of channels
     * @param  sample_rate   sample rate in Hz
     * @return               sound
     */
    Sound sound_from_xbox_adpcm(const std::byte *data, std::size_t data_length, std::size_t channel_count, std::size_t sample_rate);

    /**
     * Get the sound from 16-bit big endian PCM
     * @param  data          pointer to data
     * @param  data_length   data size
     * @param  channel_count number of channels
     * @param  sample_rate   sample rate in Hz
     * @return               sound
     */
    Sound sound_from_16_bit_pcm_big_endian(const std::byte *data, std::size_t data_length, std::size_t channel_count, std::size_t sample_rate);
    
    /**
     * Get the number of PCM samples in an Ogg Vorbis stream
     * @param  data        pointer to data
     * @param  data_length data size
     * @return             number of samples
     */
    std::size_t ogg_vorbis_sample_count(const std::byte *data, std::size_t data_length);
}

#endif
