// SPDX-License-Identifier: GPL-3.0-only

#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/sound/sound_reader.hpp>
#include <invader/sound/sound_encoder.hpp>
#include <memory>
#include <invader/file/file.hpp>
#include "wav.hpp"

namespace Invader::SoundReader {
    using namespace HEK;

    Sound sound_from_wav(const std::byte *data, std::size_t data_length) {
        Sound result = {};
        std::size_t offset = 0;

        #define READ_OR_BAIL(to_what) if(sizeof(to_what) > data_length - offset) { \
            eprintf_error("Failed to read " # to_what); \
            throw InvalidInputSoundException(); \
        } std::memcpy(reinterpret_cast<std::uint8_t *>(&to_what), data + offset, sizeof(to_what)); offset += sizeof(to_what);

        // Make sure everything is valid
        WAVChunk wav_chunk;
        READ_OR_BAIL(wav_chunk);

        if(wav_chunk.chunk_id != 0x52494646) {
            eprintf_error("WAV chunk ID is wrong");
            throw InvalidInputSoundException();
        }
        if(wav_chunk.format != 0x57415645) {
            eprintf_error("WAV chunk format is wrong");
            throw InvalidInputSoundException();
        }

        // This is what we care about
        WAVFmtSubchunk fmt_subchunk;
        READ_OR_BAIL(fmt_subchunk);

        if(fmt_subchunk.subchunk_id != 0x666D7420) {
            eprintf_error("First subchunk is not a fmt subchunk");
            throw InvalidInputSoundException();
        }
        std::size_t fmt_subchunk_size = fmt_subchunk.subchunk_size;
        std::size_t expected_fmt_subchunk_size = sizeof(WAVFmtSubchunk) - sizeof(WAVSubchunkHeader);
        if(fmt_subchunk_size < expected_fmt_subchunk_size) {
            eprintf_error("Fmt subchunk size is wrong");
            throw InvalidInputSoundException();
        }

        // Handle WAV files that are too big
        std::size_t excess_data_ignored = fmt_subchunk_size - expected_fmt_subchunk_size;
        offset += excess_data_ignored;

        // Make sure it's something we can handle
        if(fmt_subchunk.audio_format != 1 && fmt_subchunk.audio_format != 3) {
            eprintf_error("WAV data type (%u) is not integer or floating point PCM", static_cast<unsigned int>(fmt_subchunk.audio_format));
            throw InvalidInputSoundException();
        }

        // Get the values we need from the fmt header
        result.bits_per_sample = fmt_subchunk.bits_per_sample;
        result.channel_count = fmt_subchunk.channel_count;
        result.sample_rate = fmt_subchunk.sample_rate;
        result.input_bits_per_sample = fmt_subchunk.bits_per_sample;
        result.input_channel_count = fmt_subchunk.channel_count;
        result.input_sample_rate = fmt_subchunk.sample_rate;

        // Some more verification
        std::uint16_t expected_align = result.channel_count * result.bits_per_sample / 8;
        if(fmt_subchunk.block_align != expected_align) {
            eprintf_error("WAV block align value is wrong");
            throw InvalidInputSoundException();
        }
        if(result.bits_per_sample == 0 || result.bits_per_sample % 8 != 0) {
            eprintf_error("Bits per sample is zero or is not divisible by 8");
            throw InvalidInputSoundException();
        }
        if(result.sample_rate == 0) {
            eprintf_error("Sample rate is invalid");
            throw InvalidInputSoundException();
        }

        // Search for the data subchunk
        WAVSubchunkHeader subchunk = {};
        while(true) {
            READ_OR_BAIL(subchunk);
            if(subchunk.subchunk_id == 0x64617461) {
                break;
            }
            else {
                offset += subchunk.subchunk_size.read();
            }
        }

        // Convert PCM to integer
        std::size_t data_size = subchunk.subchunk_size.read();
        if(data_size > data_length) {
            eprintf_error("Data is out of bounds");
        }

        if(fmt_subchunk.audio_format == 1) {
            result.pcm = std::vector<std::byte>(data_size);
            std::memcpy(result.pcm.data(), data + offset, data_size);
        }
        else if(fmt_subchunk.audio_format == 3) {
            std::vector<float> pcm_float(data_size / sizeof(*pcm_float.data()));
            std::memcpy(result.pcm.data(), data + offset, data_size);
            result.bits_per_sample = 24;
            result.pcm = SoundEncoder::convert_float_to_int(pcm_float, 24);
        }
        else {
            std::terminate();
        }

        return result;
    }

    Sound sound_from_wav_file(const std::filesystem::path &path) {
        auto sound_data = Invader::File::open_file(path);
        if(sound_data.has_value()) {
            auto &sound_data_v = *sound_data;
            return sound_from_wav(sound_data_v.data(), sound_data_v.size());
        }
        else {
            throw FailedToOpenFileException();
        }
    }
}
