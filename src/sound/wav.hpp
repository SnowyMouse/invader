// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SOUND__WAV_HPP
#define INVADER__SOUND__WAV_HPP

#include <invader/hek/endian.hpp>

struct WAVChunk {
    Invader::HEK::BigEndian<std::uint32_t> chunk_id; // must be 'RIFF'
    Invader::HEK::LittleEndian<std::uint32_t> chunk_size; // size of the chunk (not including chunk_id and chunk_size)
    Invader::HEK::BigEndian<std::uint32_t> format; // Must be 'WAVE'
};
static_assert(sizeof(WAVChunk) == 0xC);

struct WAVSubchunkHeader {
    Invader::HEK::BigEndian<std::uint32_t> subchunk_id;
    Invader::HEK::LittleEndian<std::uint32_t> subchunk_size; // size of the subchunk not including subchunk_id and subchunk_size
};
static_assert(sizeof(WAVSubchunkHeader) == 0x8);

struct WAVFmtSubchunk : WAVSubchunkHeader {
    Invader::HEK::LittleEndian<std::uint16_t> audio_format; // 1 = PCM
    Invader::HEK::LittleEndian<std::uint16_t> channel_count; // 1 = mono, 2 = stereo
    Invader::HEK::LittleEndian<std::uint32_t> sample_rate; // Hz
    Invader::HEK::LittleEndian<std::uint32_t> byte_rate; // don't care
    Invader::HEK::LittleEndian<std::uint16_t> block_align;
    Invader::HEK::LittleEndian<std::uint16_t> bits_per_sample; // bits
};
static_assert(sizeof(WAVFmtSubchunk) == 0x10 + sizeof(WAVSubchunkHeader));

#endif
