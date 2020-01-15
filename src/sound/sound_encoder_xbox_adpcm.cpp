// SPDX-License-Identifier: GPL-3.0-only

#include <invader/sound/sound_encoder.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <memory>
#include <cstdint>

extern "C" {
#include "adpcm_xq/adpcm-lib.h"
}

namespace Invader::SoundEncoder {
    // From the MEK - I have no clue how to do this
    std::vector<std::byte> encode_to_xbox_adpcm(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::size_t channel_count) {
        // Set some parameters
        std::unique_ptr<std::vector<std::byte>> pcm_16_bit_data_ptr;
        const std::int16_t *pcm_stream;
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::size_t sample_count = pcm.size() / bytes_per_sample / channel_count;
        if(bits_per_sample != 16) {
            pcm_16_bit_data_ptr = std::make_unique<std::vector<std::byte>>(convert_int_to_int(pcm, bits_per_sample, 16));
            bytes_per_sample = 2;
            bits_per_sample = 16;
            pcm_stream = reinterpret_cast<const std::int16_t *>(pcm_16_bit_data_ptr->data());
        }
        else {
            pcm_stream = reinterpret_cast<const std::int16_t *>(pcm.data());
        }

        // Set our output
        std::vector<std::byte> adpcm_stream_buffer(sample_count * 2);
        std::uint8_t *adpcm_stream = reinterpret_cast<std::uint8_t *>(adpcm_stream_buffer.data());

        static constexpr std::size_t code_chunks_count = 8;
        std::size_t samples_per_block = code_chunks_count * 8;
        std::size_t block_count = sample_count / samples_per_block;
        std::size_t num_bytes_decoded = 0;

        std::size_t pcm_block_size   = samples_per_block * channel_count;  // number of pcm sint16 per block
        std::size_t adpcm_block_size = (code_chunks_count * 4 + 4) * channel_count;  // number of adpcm bytes per block

        std::int32_t average_deltas[2];
        void *adpcm_context = NULL;

        // calculate initial adpcm predictors using decaying average
        for (std::size_t c = 0; c < channel_count; c++) {
            average_deltas[c] = 0;
            for (std::size_t i = c + pcm_block_size - channel_count; i >= channel_count; i -= channel_count) {
                average_deltas[c] = (average_deltas[c] / 8) + std::abs(static_cast<std::int32_t>(pcm_stream[i]) - pcm_stream[i - channel_count]);
            }
            average_deltas[c] /= 8;
        }

        // Encode!
        adpcm_context = adpcm_create_context(channel_count, 3, 0, average_deltas);
        for (std::size_t b = 0; b < block_count; b++) {
            adpcm_encode_block(adpcm_context, adpcm_stream, &num_bytes_decoded, pcm_stream, samples_per_block);
            adpcm_stream += adpcm_block_size;
            pcm_stream += pcm_block_size;
        }
        adpcm_free_context(adpcm_context);

        // Resize to fit the buffer
        adpcm_stream_buffer.resize(reinterpret_cast<std::byte *>(adpcm_stream) - adpcm_stream_buffer.data());
        return adpcm_stream_buffer;
    }
}
