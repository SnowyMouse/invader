// SPDX-License-Identifier: GPL-3.0-only

#include <invader/sound/sound_encoder.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/error.hpp>
#include <vorbis/vorbisenc.h>
#include <memory>
#include <variant>
#include <cstdint>

namespace Invader::SoundEncoder {
    static std::vector<std::byte> encode_to_ogg_vorbis(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate, std::variant<float, std::uint16_t> vorbis_quality) {
        // Begin
        std::vector<std::byte> output_samples;
        std::size_t bytes_per_sample_one_channel = bits_per_sample / 8;
        std::size_t split_sample_count = pcm.size() / bytes_per_sample_one_channel;
        std::size_t split_effective_sample_count = split_sample_count / channel_count;
        std::vector<float> float_samples = SoundEncoder::convert_int_to_float(pcm, bytes_per_sample_one_channel * 8);

        vorbis_info vi;
        vorbis_info_init(&vi);
        int ret;
        
        switch(vorbis_quality.index()) {
            case 0:
                if((ret = vorbis_encode_init_vbr(&vi, channel_count, sample_rate, std::get<0>(vorbis_quality)))) {
                    eprintf_error("Failed to initialize vorbis encoder (invalid parameters given?)");
                    vorbis_info_clear(&vi);
                    throw SoundEncodeFailureException();
                }
                break;
            case 1: {
                int bitrate = static_cast<int>(std::get<1>(vorbis_quality)) * 1000;
                if((ret = vorbis_encode_init(&vi, channel_count, sample_rate, bitrate, bitrate, bitrate))) {
                    if(ret == OV_EIMPL) {
                        eprintf_error("Failed to initialize vorbis encoder (bitrate likely too low or too high)");
                    }
                    else {
                        eprintf_error("Failed to initialize vorbis encoder (invalid parameters given?)");
                    }
                    vorbis_info_clear(&vi);
                    throw SoundEncodeFailureException();
                }
                break;
            }
            default:
                ret = 0;
                break;
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
    
    std::vector<std::byte> encode_to_ogg_vorbis_vbr(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate, float vorbis_quality) {
        return encode_to_ogg_vorbis(pcm, bits_per_sample, channel_count, sample_rate, vorbis_quality);
    }
    
    std::vector<std::byte> encode_to_ogg_vorbis_cbr(const std::vector<std::byte> &pcm, std::size_t bits_per_sample, std::uint32_t channel_count, std::uint32_t sample_rate, std::uint16_t vorbis_bitrate) {
        return encode_to_ogg_vorbis(pcm, bits_per_sample, channel_count, sample_rate, vorbis_bitrate);
    }
}
