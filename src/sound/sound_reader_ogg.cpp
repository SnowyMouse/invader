// SPDX-License-Identifier: GPL-3.0-only

#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/file/file.hpp>
#include <invader/sound/sound_encoder.hpp>
#include <invader/sound/sound_reader.hpp>
#include <FLAC/stream_decoder.h>
#include <vorbis/vorbisenc.h>
#include <memory>

#include <algorithm>

namespace Invader::SoundReader {
    Sound sound_from_ogg_file(const char *path) {
        auto sound_data = Invader::File::open_file(path);
        if(sound_data.has_value()) {
            auto &sound_data_v = *sound_data;
            return sound_from_ogg(sound_data_v.data(), sound_data_v.size());
        }
        else {
            throw FailedToOpenFileException();
        }
    }

    Sound sound_from_ogg(const std::byte *data, std::size_t data_length) {
        Sound result = {};
        result.bits_per_sample = 24;
        std::vector<float> pcm_float;

        // Let's begin.
        std::size_t offset = 0;
        ogg_sync_state oy;
        ogg_sync_init(&oy);

        #define READ_ADVANCE(where, amount) if(offset + amount > data_length) { eprintf_error("Tried to read out of bounds"); throw InvalidInputSoundException(); } std::memcpy(where, data + offset, amount); offset += amount;

        while(true) {
            #define BUFFER_SIZE static_cast<std::size_t>(4096)
            #define REMAINING_DATA (data_length - offset)
            char *buffer = ogg_sync_buffer(&oy, BUFFER_SIZE);
            std::size_t bytes = std::min(REMAINING_DATA, BUFFER_SIZE);
            READ_ADVANCE(buffer, bytes);
            ogg_sync_wrote(&oy, bytes);

            // Get pages
            ogg_page og;
            if(ogg_sync_pageout(&oy, &og) != 1) {
               // See if we hit the end of the buffer
               if(bytes < 4096) {
                   break;
               }

               // Error otherwise
               eprintf_error("Ogg file is invalid");
               throw InvalidInputSoundException();
            }

            // Let's continue
            ogg_stream_state os;
            ogg_stream_init(&os, ogg_page_serialno(&og));

            // Now, let's see if we can get this going
            if(ogg_stream_pagein(&os, &og) < 0){
                eprintf_error("Failed to read an Ogg page (Ogg file may be invalid)");
                throw InvalidInputSoundException();
            }
            ogg_packet op;
            if(ogg_stream_packetout(&os, &op) != 1){
                eprintf_error("Ogg file is invalid");
                throw InvalidInputSoundException();
            }

            // Next, let's see if we're Vorbis
            vorbis_info vi;
            vorbis_comment vc;
            vorbis_info_init(&vi);
            vorbis_comment_init(&vc);

            try {
                if(vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
                    // Not Vorbis
                    eprintf_error("Only Vorbis data is supported for now in Ogg files");
                    throw InvalidInputSoundException();
                }

                result.channel_count = vi.channels;
                result.sample_rate = vi.rate;
                result.input_channel_count = result.channel_count;
                result.input_sample_rate = result.sample_rate;

                int i = 0;
                int eos = 0;

                while(i < 2) {
                    while(i < 2) {
                        int result = ogg_sync_pageout(&oy,&og);
                        if(result == 0) {
                            break; // moar bytes plz
                        }

                        // Some reading & error checking
                        if(result == 1) {
                            ogg_stream_pagein(&os, &og);
                            while(i < 2) {
                                result = ogg_stream_packetout(&os, &op);
                                if(result == 0) {
                                    break;
                                }
                                if(result < 0) {
                                    eprintf_error("Secondary header is invalid")
                                    throw InvalidInputSoundException();
                                }
                                result = vorbis_synthesis_headerin(&vi, &vc, &op);
                                if(result < 0) {
                                    eprintf_error("Secondary header is invalid")
                                    throw InvalidInputSoundException();
                                }
                                i++;
                            }
                        }
                    }

                    buffer = ogg_sync_buffer(&oy, BUFFER_SIZE);
                    bytes = std::min(REMAINING_DATA, BUFFER_SIZE);
                    READ_ADVANCE(buffer, bytes);
                    if(bytes == 0 && i < 2) {
                        eprintf_error("Ogg file ended before all headers were read");
                        throw InvalidInputSoundException();
                    }
                    ogg_sync_wrote(&oy,bytes);
                }

                std::size_t convsize = BUFFER_SIZE / vi.channels;

                vorbis_dsp_state vd;
                vorbis_block vb;

                // Now start decoding
                if(vorbis_synthesis_init(&vd, &vi) == 0) {
                    vorbis_block_init(&vd, &vb);

                    // Do this until we're done
                    try {
                        while(!eos) {
                            while(!eos) {
                                int result = ogg_sync_pageout(&oy, &og);
                                if(result == 0) {
                                    break;
                                }
                                if(result < 0) {
                                    eprintf_error("Ogg file has missing or corrupt data");
                                    throw InvalidInputSoundException();
                                }
                                else {
                                    ogg_stream_pagein(&os, &og);
                                    while(true) {
                                        result = ogg_stream_packetout(&os, &op);
                                        if(result == 0) {
                                            break; // moooar
                                        }

                                        if(result < 0) {
                                            eprintf_error("Ogg file has missing or corrupt data");
                                            throw InvalidInputSoundException();
                                        }
                                        else {
                                            // Now for the good part
                                            float **pcm;
                                            int samples;
                                            if(vorbis_synthesis(&vb, &op) == 0) {
                                                vorbis_synthesis_blockin(&vd, &vb);
                                            }

                                            // Loopdeedoo
                                            while((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
                                                std::size_t bout = std::min(static_cast<std::size_t>(samples), convsize);
                                                for(std::size_t j = 0; j < bout; j++) {
                                                    for(i = 0; i < vi.channels; i++) {
                                                        pcm_float.emplace_back(pcm[i][j]);
                                                    }
                                                }
                                                vorbis_synthesis_read(&vd, bout);
                                            }
                                        }
                                    }

                                    if(ogg_page_eos(&og)) {
                                        eos = 1;
                                    }
                                }
                            }

                            if(!eos) {
                                bytes = std::min(REMAINING_DATA, BUFFER_SIZE);
                                buffer = ogg_sync_buffer(&oy, BUFFER_SIZE);
                                READ_ADVANCE(buffer, bytes);
                                ogg_sync_wrote(&oy, bytes);
                                if(bytes == 0) {
                                    eos = 1;
                                }
                            }
                        }
                    }
                    catch(std::exception &) {
                        vorbis_block_clear(&vb);
                        vorbis_dsp_clear(&vd);
                        throw;
                    }

                    vorbis_block_clear(&vb);
                    vorbis_dsp_clear(&vd);
                }
                else {
                    eprintf_error("Invalid or corrupt Ogg header");
                    throw InvalidInputSoundException();
                }
            }
            catch(std::exception &) {
                ogg_stream_clear(&os);
                vorbis_comment_clear(&vc);
                vorbis_info_clear(&vi);
                throw;
            }

            // Clean up
            ogg_stream_clear(&os);
            vorbis_comment_clear(&vc);
            vorbis_info_clear(&vi);
        }

        result.pcm = SoundEncoder::convert_float_to_int(pcm_float, 24);
        result.input_bits_per_sample = 32;
        result.bits_per_sample = 24;

        return result;
    }
}