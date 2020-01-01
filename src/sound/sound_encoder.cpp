// SPDX-License-Identifier: GPL-3.0-only

#include <invader/sound/sound_encoder.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/error.hpp>
#include <vorbis/vorbisenc.h>
#include <memory>
#include <cstdint>
#include <samplerate.h>

extern "C" {
#include "adpcm_xq/adpcm-lib.h"
}

namespace Invader::SoundEncoder {
    std::int32_t read_sample(const std::byte *pcm, std::size_t bits_per_sample) noexcept {
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::int32_t sample_value = 0;

        // Get the sample value
        for(std::size_t b = 0; b < bytes_per_sample; b++) {
            std::int32_t significance = 8 * b;
            if(b + 1 == bytes_per_sample) {
                sample_value |= static_cast<std::int64_t>(static_cast<std::int8_t>(pcm[b])) << significance;
            }
            else {
                sample_value |= static_cast<std::int64_t>(static_cast<std::uint8_t>(pcm[b])) << significance;
            }
        }

        return sample_value;
    }

    std::vector<std::byte> encode_to_ogg_vorbis(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate, float vorbis_quality) {
        // Begin
        std::vector<std::byte> output_samples;
        std::size_t bytes_per_sample_one_channel = bits_per_sample / 8;
        std::size_t split_sample_count = pcm.size() / bytes_per_sample_one_channel;
        std::size_t split_effective_sample_count = split_sample_count / channel_count;
        std::vector<float> float_samples = SoundEncoder::convert_int_to_float(pcm, bytes_per_sample_one_channel * 8);

        vorbis_info vi;
        vorbis_info_init(&vi);
        auto ret = vorbis_encode_init_vbr(&vi, channel_count, sample_rate, vorbis_quality);
        if(ret) {
            eprintf_error("Failed to initialize vorbis encoder");
            throw SoundEncodeFailureException();
        }

        // Set the comment
        vorbis_comment vc;
        vorbis_comment_init(&vc);
        vorbis_comment_add_tag(&vc, "ENCODER", full_version());

        // Start making a vorbis block
        vorbis_dsp_state vd;
        vorbis_block vb;
        vorbis_analysis_init(&vd, &vi);
        vorbis_block_init(&vd, &vb);

        // Ogg packet stuff
        ogg_packet op;
        ogg_packet op_comment;
        ogg_packet op_code;
        ogg_page og;
        ogg_stream_state os;
        ogg_stream_init(&os, 0);
        vorbis_analysis_headerout(&vd, &vc, &op, &op_comment, &op_code);
        ogg_stream_packetin(&os, &op);
        ogg_stream_packetin(&os, &op_comment);
        ogg_stream_packetin(&os, &op_code);

        // Do stuff until we don't do stuff anymore since we need the data on a separate page
        while(ogg_stream_flush(&os, &og)) {
            output_samples.insert(output_samples.end(), reinterpret_cast<std::byte *>(og.header), reinterpret_cast<std::byte *>(og.header) + og.header_len);
            output_samples.insert(output_samples.end(), reinterpret_cast<std::byte *>(og.body), reinterpret_cast<std::byte *>(og.body) + og.body_len);
        }

        // Analyze data
        static constexpr std::size_t SPLIT_COUNT = 1024;
        std::size_t encoded_count = 0;

        // Loop until we're done
        bool eos = false;
        std::size_t samples_read = 0;
        while(!eos) {
            // Subtract the sample count minus the number of samples read (we can and will get 0 here, too - this is intentional)
            std::size_t sample_count_to_encode = (split_effective_sample_count - samples_read);
            float **buffer = vorbis_analysis_buffer(&vd, sample_count_to_encode);

            // Make sure we don't read more than SPLIT_COUNT, since libvorbis can segfault if we read too much at once.
            if(sample_count_to_encode > SPLIT_COUNT) {
                sample_count_to_encode = SPLIT_COUNT;
            }

            // Load each sample
            for(std::size_t i = 0; i < sample_count_to_encode; i++) {
                auto *sample = float_samples.data() + (i + encoded_count) * channel_count;
                for(std::size_t c = 0; c < channel_count; c++) {
                    buffer[c][i] = sample[c];
                }
            }

            // Set how many samples we wrote
            ret = vorbis_analysis_wrote(&vd, sample_count_to_encode);
            if(ret) {
                eprintf_error("Failed to read samples");
                std::exit(EXIT_FAILURE);
            }

            // Encode the blocks
            while(vorbis_analysis_blockout(&vd, &vb) == 1) {
                vorbis_analysis(&vb, nullptr);
                vorbis_bitrate_addblock(&vb);
                while(vorbis_bitrate_flushpacket(&vd, &op)) {
                    ogg_stream_packetin(&os, &op);
                    while(!eos) {
                        // Write data if we have a page
                        if(!ogg_stream_pageout(&os, &og)) {
                            break;
                        }

                        output_samples.insert(output_samples.end(), reinterpret_cast<std::byte *>(og.header), reinterpret_cast<std::byte *>(og.header) + og.header_len);
                        output_samples.insert(output_samples.end(), reinterpret_cast<std::byte *>(og.body), reinterpret_cast<std::byte *>(og.body) + og.body_len);

                        // End if we need to
                        if(ogg_page_eos(&og)) {
                            eos = true;
                        }
                    }
                }
            }

            // Increment
            encoded_count += sample_count_to_encode;
            samples_read += sample_count_to_encode;
        }

        // Clean up
        ogg_stream_clear(&os);
        vorbis_block_clear(&vb);
        vorbis_dsp_clear(&vd);
        vorbis_comment_clear(&vc);
        vorbis_info_clear(&vi);

        output_samples.shrink_to_fit();
        return output_samples;
    }

    // From the MEK - I have no clue how to do this
    std::vector<std::byte> encode_to_xbox_adpcm(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::size_t channel_count) {
        // Set some parameters
        std::unique_ptr<std::vector<std::byte>> pcm_16_bit_data_ptr;
        const std::int16_t *pcm_stream;
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::size_t sample_count = pcm.size() / bytes_per_sample / channel_count;
        if(true) {
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

    std::vector<std::byte> convert_to_16_bit_pcm_big_endian(const std::vector<std::byte> &pcm, std::size_t bits_per_sample) {
        // Convert to 16 bits per sample if needed
        std::vector<std::byte> pcm_to_use;
        if(bits_per_sample != 16) {
            pcm_to_use = convert_int_to_int(pcm, bits_per_sample, 16);
        }
        else {
            pcm_to_use = pcm;
        }

        // Swap endianness
        std::uint16_t *samples = reinterpret_cast<std::uint16_t *>(pcm_to_use.data());
        std::size_t sample_count = pcm_to_use.size() / sizeof(*samples);
        for(std::size_t i = 0; i < sample_count; i++) {
            auto sample = samples[i];
            auto *sample_bytes = reinterpret_cast<std::byte *>(samples + i);
            sample_bytes[1] = static_cast<std::byte>(sample & 0xFF);
            sample_bytes[0] = static_cast<std::byte>((sample >> 8) & 0xFF);
        }

        // Done!
        return pcm_to_use;
    }

    std::vector<std::byte> convert_int_to_int(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::size_t new_bits_per_sample) {
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::size_t new_bytes_per_sample = new_bits_per_sample / 8;
        std::size_t sample_count = pcm.size() / bytes_per_sample;
        std::vector<std::byte> samples(sample_count * new_bytes_per_sample);

        // Calculate what we divide by
        std::int64_t divide_by = 1 << bits_per_sample;

        // Calculate what we multiply by
        std::int64_t multiply_by = 1 << new_bits_per_sample;

        auto *pcm_data = pcm.data();
        auto *output_pcm_data = samples.data();

        for(std::size_t i = 0; i < sample_count; i++) {
            // Get the new sample value
            std::int64_t sample = read_sample(pcm_data, bits_per_sample);
            std::int64_t new_sample = sample * multiply_by / divide_by;
            write_sample(static_cast<std::int32_t>(new_sample), output_pcm_data, new_bits_per_sample);

            // Add it
            pcm_data += bytes_per_sample;
            output_pcm_data += new_bytes_per_sample;
        }

        return samples;
    }

    std::vector<float> convert_int_to_float(const std::vector<std::byte> &pcm, std::size_t bits_per_sample) {
        std::vector<float> samples;
        std::size_t bytes_per_sample = bits_per_sample / 8;
        std::size_t sample_count = pcm.size() / bytes_per_sample;
        samples.reserve(sample_count);

        // Calculate what we divide by
        float divide_by = (1 << bits_per_sample) / 2.0F;
        float divide_by_minus_one = divide_by - 1;
        float divide_by_arr[2] = { divide_by_minus_one, divide_by };

        auto *pcm_data = pcm.data();

        for(std::size_t i = 0; i < sample_count; i++) {
            std::int64_t sample = read_sample(pcm_data, bits_per_sample);
            samples.emplace_back(sample / divide_by_arr[sample < 0]);
            pcm_data += bytes_per_sample;
        }

        return samples;
    }

    std::vector<std::byte> convert_float_to_int(const std::vector<float> &pcm, std::size_t new_bits_per_sample) {
        std::size_t sample_count = pcm.size();
        std::size_t bytes_per_sample = new_bits_per_sample / 8;
        std::vector<std::byte> samples(sample_count * bytes_per_sample);
        auto *sample_data = samples.data();

        // Calculate what we multiply by
        std::int64_t multiply_by = (1 << new_bits_per_sample) / 2.0;
        std::int64_t multiply_by_minus_one = multiply_by - 1;
        std::int64_t multiply_by_arr[2] = { multiply_by_minus_one, multiply_by };

        for(std::size_t i = 0; i < sample_count; i++) {
            std::int64_t sample = pcm[i] * multiply_by_arr[pcm[i] < 0];
            write_sample(static_cast<std::int32_t>(sample), sample_data, new_bits_per_sample);
            sample_data += bytes_per_sample;
        }

        return samples;
    }

    void write_sample(std::int32_t sample, std::byte *pcm, std::size_t bits_per_sample) noexcept {
        std::size_t bytes_per_sample = bits_per_sample / 8;
        for(std::size_t b = 0; b < bytes_per_sample; b++) {
            pcm[b] = static_cast<std::byte>((sample >> b * 8) & 0xFF);
        }
    }
}
