// SPDX-License-Identifier: GPL-3.0-only

#include <invader/sound/sound_encoder.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <memory>
#include <cstdint>
#include "wav.hpp"

namespace Invader::SoundEncoder {
    std::vector<std::byte> convert_to_pcm_wav(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate) {
        // Get our chunks
        WAVChunk wav_chunk = {};
        WAVSubchunkHeader data = {};
        WAVFmtSubchunk fmt = {};

        // Write wav chunk
        wav_chunk.chunk_id = 0x52494646;
        wav_chunk.format = 0x57415645;
        wav_chunk.chunk_size = pcm.size() + sizeof(WAVSubchunkHeader) + sizeof(WAVFmtSubchunk) + sizeof(wav_chunk.format);

        // Write fmt
        fmt.subchunk_size = sizeof(fmt) - sizeof(WAVSubchunkHeader);
        fmt.subchunk_id = 0x666D7420;
        fmt.audio_format = 1;
        fmt.bits_per_sample = bits_per_sample;
        fmt.sample_rate = sample_rate;
        fmt.channel_count = channel_count;
        std::size_t bytes_per_sample = bits_per_sample / 8;
        fmt.block_align = channel_count * bytes_per_sample;
        fmt.byte_rate = channel_count * bytes_per_sample * sample_rate;

        // Write data chunk
        data.subchunk_id = 0x64617461;
        data.subchunk_size = pcm.size();

        // Write
        std::vector<std::byte> all_data;
        #define INSERT_THING(what) all_data.insert(all_data.end(), reinterpret_cast<const std::byte *>(&what), reinterpret_cast<const std::byte *>(&what + 1));
        INSERT_THING(wav_chunk);
        INSERT_THING(fmt);
        INSERT_THING(data);
        all_data.insert(all_data.end(), pcm.begin(), pcm.end());

        return all_data;
    }
}
