// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/endian.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/sound/sound_reader.hpp>

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

    SoundReader::Sound SoundReader::sound_from_wav(const std::byte *data, std::size_t data_length) {
        Sound result = {};

        #define INCREMENT_DATA_PTR(amt) data += amt; data_length -= amt;

        // Make sure we can read it
        if(data_length < sizeof(WAVChunk)) {
            throw;
        }

        // Make sure everything is valid
        const auto &wav_chunk = *reinterpret_cast<const WAVChunk *>(data);
        INCREMENT_DATA_PTR(sizeof(WAVChunk));

        if(wav_chunk.chunk_id != 0x52494646) {
            eprintf_error("WAV chunk ID is wrong");
            throw InvalidInputSoundException();
        }
        if(wav_chunk.chunk_size > data_length + sizeof(std::uint32_t)) {
            eprintf_error("WAV data length is wrong");
            throw InvalidInputSoundException();
        }
        if(wav_chunk.format != 0x57415645) {
            eprintf_error("WAV chunk format is wrong");
            throw InvalidInputSoundException();
        }

        // This is what we care about
        data_length = wav_chunk.chunk_size - sizeof(std::uint32_t);
        if(data_length < sizeof(WAVFmtSubchunk)) {
            eprintf_error("WAV does not have enough data to hold a fmt subchunk");
            throw InvalidInputSoundException();
        }
        const auto &fmt_subchunk = *reinterpret_cast<const WAVFmtSubchunk *>(data);
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
        INCREMENT_DATA_PTR(sizeof(fmt_subchunk));
        if(data_length < sizeof(WAVSubchunkHeader)) {
            eprintf_error("WAV is too small to hold a data subchunk");
            throw InvalidInputSoundException();
        }
        const auto &data_subchunk = *reinterpret_cast<const WAVSubchunkHeader *>(data);
        INCREMENT_DATA_PTR(sizeof(data_subchunk));
        if(data_subchunk.subchunk_id != 0x64617461) {
            eprintf_error("Second subchunk is not a data subchunk");
            throw InvalidInputSoundException();
        }
        if(data_length < data_subchunk.subchunk_size) {
            eprintf_error("Data subchunk size is wrong");
            throw InvalidInputSoundException();
        }
        result.pcm = std::vector<std::byte>(data, data + data_subchunk.subchunk_size);

        return result;
    }
}
