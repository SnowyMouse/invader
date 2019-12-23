// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/endian.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/sound/sound_reader.hpp>
#include <FLAC/stream_decoder.h>

namespace Invader {
    using namespace HEK;

    struct WAVChunk {
        BigEndian<std::uint32_t> chunk_id; // must be 'RIFF'
        LittleEndian<std::uint32_t> chunk_size; // size of the chunk (not including chunk_id and chunk_size)
        BigEndian<std::uint32_t> format; // Must be 'WAVE'
    };
    static_assert(sizeof(WAVChunk) == 0xC);

    struct WAVSubchunkHeader {
        BigEndian<std::uint32_t> subchunk_id;
        LittleEndian<std::uint32_t> subchunk_size; // size of the subchunk not including subchunk_id and subchunk_size
    };
    static_assert(sizeof(WAVSubchunkHeader) == 0x8);

    struct WAVFmtSubchunk : WAVSubchunkHeader {
        LittleEndian<std::uint16_t> audio_format; // 1 = PCM
        LittleEndian<std::uint16_t> channel_count; // 1 = mono, 2 = stereo
        LittleEndian<std::uint32_t> sample_rate; // Hz
        LittleEndian<std::uint32_t> byte_rate; // don't care
        LittleEndian<std::uint16_t> block_align;
        LittleEndian<std::uint16_t> bits_per_sample; // bits
    };
    static_assert(sizeof(WAVFmtSubchunk) == 0x10 + sizeof(WAVSubchunkHeader));

    SoundReader::Sound SoundReader::sound_from_wav(const char *path) {
        Sound result = {};

        #define READ_OR_BAIL(to_what) if(std::fread(&to_what, sizeof(to_what), 1, file) != 1) { \
            eprintf_error("Failed to read " # to_what); \
            throw InvalidInputSoundException(); \
        }

        // Open it
        std::FILE *file = nullptr;
        try {
            file = std::fopen(path, "rb");
            if(!file) {
                eprintf_error("Failed to open %s", path);
                throw InvalidInputSoundException();
            }

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
            if(fmt_subchunk.subchunk_size != sizeof(WAVFmtSubchunk) - sizeof(WAVSubchunkHeader)) {
                eprintf_error("Fmt subchunk size is wrong");
                throw InvalidInputSoundException();
            }
            if(fmt_subchunk.audio_format != 1) {
                eprintf_error("WAV data is not PCM");
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

            // Get the data
            WAVSubchunkHeader data_subchunk;
            READ_OR_BAIL(data_subchunk);

            result.pcm = std::vector<std::byte>(data_subchunk.subchunk_size);
            if(std::fread(result.pcm.data(), data_subchunk.subchunk_size, 1, file) != 1) {
                eprintf_error("Failed to read data");
                throw InvalidInputSoundException();
            }
        }
        catch(std::exception &) {
            std::fclose(file);
            throw;
        }

        // Close it
        std::fclose(file);

        return result;
    }

    SoundReader::Sound SoundReader::sound_from_flac(const char *path) {
        Sound result = {};
        eprintf_error("Unimplemented");
        std::exit(1);
        return result;
    }
}
