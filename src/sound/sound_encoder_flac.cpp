// SPDX-License-Identifier: GPL-3.0-only

#include <invader/sound/sound_encoder.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <FLAC/stream_encoder.h>
#include <memory>
#include <cstdint>

namespace Invader::SoundEncoder {
    struct FLACHolder {
        std::vector<std::byte> flac;
        std::size_t offset;
    };

    static FLAC__StreamEncoderWriteStatus write_flac_data(const FLAC__StreamEncoder *, const FLAC__byte buffer[], size_t bytes, uint32_t, uint32_t, void *client_data) noexcept {
        auto *holder = reinterpret_cast<FLACHolder *>(client_data);
        std::size_t end = holder->offset + bytes;
        if(end > holder->flac.size()) {
            holder->flac.resize(end);
        }
        std::memcpy(holder->flac.data() + holder->offset, buffer, bytes);
        holder->offset += bytes;
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }

    static FLAC__StreamEncoderSeekStatus seek_flac_data(const FLAC__StreamEncoder *, FLAC__uint64 absolute_byte_offset, void *client_data) noexcept {
        reinterpret_cast<FLACHolder *>(client_data)->offset = static_cast<std::size_t>(absolute_byte_offset);
        return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
    }

    static FLAC__StreamEncoderTellStatus tell_flac_data(const FLAC__StreamEncoder *, FLAC__uint64 *absolute_byte_offset, void *client_data) noexcept {
        *absolute_byte_offset = static_cast<FLAC__uint64>(reinterpret_cast<FLACHolder *>(client_data)->offset);
        return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
    }

    std::vector<std::byte> encode_to_flac(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate, std::uint32_t compression_level) {
        // First make our buffer
        std::vector<FLAC__int32> buffer;
        const auto *pcm_data = pcm.data();
        std::size_t bytes_per_sample = bits_per_sample / 8;
        if(pcm.size() % bytes_per_sample * channel_count != 0) {
            eprintf_error("PCM data size is not divisible by channel count * bytes per sample");
            throw InvalidInputSoundException();
        }
        std::size_t pcm_size = pcm.size();
        std::size_t sample_count = pcm_size / bytes_per_sample;
        buffer.reserve(sample_count);
        for(std::size_t s = 0; s < sample_count; s++) {
            buffer.emplace_back(static_cast<FLAC__int32>(read_sample(pcm_data + bytes_per_sample * s, bits_per_sample)));
        }

        FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();

        // Set our FLAC holder 9000
        FLACHolder holder = { };

        // Set our metadata
        FLAC__stream_encoder_set_bits_per_sample(encoder, bits_per_sample);
        FLAC__stream_encoder_set_channels(encoder, channel_count);
        FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
        FLAC__stream_encoder_set_compression_level(encoder, compression_level);

        try {
            if(FLAC__stream_encoder_init_stream(encoder, write_flac_data, seek_flac_data, tell_flac_data, NULL, &holder) != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
                eprintf_error("Failed to init FLAC stream");
                throw InvalidInputSoundException();
            }

            if(!FLAC__stream_encoder_process_interleaved(encoder, buffer.data(), static_cast<std::size_t>(buffer.size()) / channel_count)) {
                eprintf_error("Failed to encode PCM stream");
                throw InvalidInputSoundException();
            }

            FLAC__stream_encoder_finish(encoder);
            FLAC__stream_encoder_delete(encoder);
        }
        catch(std::exception &) {
            FLAC__stream_encoder_delete(encoder);
            throw;
        }

        return holder.flac;
    }
}
