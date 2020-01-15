// SPDX-License-Identifier: GPL-3.0-only

#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/sound/sound_reader.hpp>

extern "C" {
#include "adpcm_xq/adpcm-lib.h"
}

const static int ADPCM_STEP_TABLE_MAX_INDEX = sizeof(step_table) / sizeof(step_table[0]) - 1;
const static int XBOX_ADPCM_ENCODED_BLOCKSIZE = 36;
const static int XBOX_ADPCM_DECODED_BLOCKSIZE = 128;

namespace Invader::SoundReader {
    static void decode_xbadpcm_stream(std::byte *pcm_stream_buf, const std::byte *adpcm_stream_buf, std::size_t adpcm_stream_buf_len, std::uint8_t channel_count, std::uint32_t code_chunks_count);

    Sound sound_from_xbox_adpcm(const std::byte *data, std::size_t data_length, std::size_t channel_count, std::size_t sample_rate) {
        Sound result = {};

        if(channel_count > 2 || channel_count < 1) {
            eprintf_error("Only mono or stereo streams are supported");
            throw InvalidInputSoundException();
        }

        // Set parameters
        result.input_bits_per_sample = 16;
        result.bits_per_sample = 16;
        result.channel_count = channel_count;
        result.input_channel_count = result.channel_count;
        result.sample_rate = sample_rate;
        result.input_sample_rate = result.sample_rate;

        std::size_t block_count = static_cast<int>(data_length / (channel_count * XBOX_ADPCM_ENCODED_BLOCKSIZE));
        if(data_length % XBOX_ADPCM_ENCODED_BLOCKSIZE != 0) {
            eprintf_error("Data length is not divisible by Xbox ADPCM block size");
            throw InvalidInputSoundException();
        }

        // Do it!
        result.pcm = std::vector<std::byte>(block_count * XBOX_ADPCM_DECODED_BLOCKSIZE);
        decode_xbadpcm_stream(result.pcm.data(), data, data_length, static_cast<std::uint8_t>(channel_count), 8);

        // Return the result
        return result;
    }

    // From Refinery (by MosesofEgypt)

    // Description of ADPCM stream block:
    //   The stream starts with a 4 byte chunk for each audio channel:
    //     bytes 0-2: the initial 16bit pcm sample(little endian std::int16_t)
    //     byte 2:    the initial step table index
    //       NOTE: do a CLAMP(0, ADPCM_STEP_TABLE_SIZE))
    //     byte 3: reserved(usually left at 0)
    //
    //   The rest of the stream is alternating chunks of adpcm codes for each channel.
    //   Each chunk is 4 bytes, and contains 8 codes. The code bytes are sequential,
    //   and the first nibble of each byte is the first code. So the stream will look
    //   like this:
    //     channel 0: (left)   b0   b1   b2   b3
    //     channel 1: (right)  b4   b5   b6   b7
    //     channel 0: (left)   b8   b9   b10  b11
    //     channel 1: (right)  b12  b13  b14  b15
    //     cont...

    typedef struct {
        std::int16_t pcm_sample;  /* the current decoded adpcm sample. When calling
        **                     decode_adpcm_sample this is supposed to be the
        **                     predictor for decoding the next sample. Contains
        **                     the decoded sample when the function returns.*/
        std::int8_t index;  /* An index into step_table.
        **               Used for calculating the current differential step.
        **               Contains the next step index when the function returns.*/
        std::uint8_t code;  /* An index into index_table.
        **              The 4bit adpcm code for calculating the next differential index.
        **              Update this before calling decode_adpcm_sample.*/
    } AdpcmState;


    /* This function will decode the next adpcm sample given as an AdpcmState struct.
    This function accepts and returns the whole struct since it can easily fit in a
    single 32bit register, and should be more efficient than passing a pointer to a struct.
    */
    static AdpcmState decode_adpcm_sample(AdpcmState state) {
        int delta, step_size = step_table[state.index];
        int result = state.pcm_sample;  /* pcm_sample could over/underflow in the code below,
        **                                 so we keep the result as an int for clamping.*/

        delta = step_size >> 3;
        if (state.code & 4) delta += step_size;
        if (state.code & 2) delta += step_size >> 1;
        if (state.code & 1) delta += step_size >> 2;
        if (state.code & 8) delta = -delta;

        result += delta;

        if (result >= 32767)
            state.pcm_sample = 32767;
        else if (result <= -32768)
            state.pcm_sample = -32768;
        else
            state.pcm_sample = static_cast<std::int16_t>(result);

        state.index += index_table[state.code];
        if (state.index < 0)
            state.index = 0;
        else if (state.index > ADPCM_STEP_TABLE_MAX_INDEX)
            state.index = ADPCM_STEP_TABLE_MAX_INDEX;

        return state;
    }

    static void decode_xbadpcm_stream(std::byte *pcm_stream_buf, const std::byte *adpcm_stream_buf, std::size_t adpcm_stream_buf_len, std::uint8_t channel_count, std::uint32_t code_chunks_count) {
        AdpcmState adpcm_states[MAX_AUDIO_CHANNEL_COUNT];
        int block_count = static_cast<int>(
            adpcm_stream_buf_len /
            (channel_count * (4 + 4 * code_chunks_count)));
        const std::uint8_t *adpcm_stream = reinterpret_cast<const std::uint8_t *>(adpcm_stream_buf);
        std::int16_t *pcm_stream = reinterpret_cast<std::int16_t *>(pcm_stream_buf);
        std::uint32_t codes;
        std::uint32_t samples_per_chunk = 8 * channel_count;
        std::uint32_t samples_this_chunk = 0, samples_remaining_this_block = 0;
        std::uint32_t samples_per_block = samples_per_chunk * code_chunks_count;

        for (int b = 0; b < block_count; b++) {
            samples_remaining_this_block = samples_per_block;
            // initialize the adpcm state structs
            for (int c = 0; c < channel_count; c++) {
                adpcm_states[c].pcm_sample = adpcm_stream[0] | (adpcm_stream[1] << 8);
                adpcm_states[c].index = adpcm_stream[2];
                adpcm_stream += 4;

                pcm_stream[0] = adpcm_states[c].pcm_sample;
                pcm_stream++;
                samples_remaining_this_block--;

                if (adpcm_states[c].index < 0)
                    adpcm_states[c].index = 0;
                else if (adpcm_states[c].index > ADPCM_STEP_TABLE_MAX_INDEX)
                    adpcm_states[c].index = ADPCM_STEP_TABLE_MAX_INDEX;
            }

            while (samples_remaining_this_block) {
                // loop over each channel in each chunk
                if (samples_remaining_this_block < samples_per_chunk)
                    samples_this_chunk = samples_remaining_this_block;
                else
                    samples_this_chunk = samples_per_chunk;

                for (std::size_t c = 0; c < channel_count; c++) {
                    // OR the codes together for easy access
                    codes = (
                         adpcm_stream[0] |
                        (adpcm_stream[1] << 8) |
                        (adpcm_stream[2] << 16) |
                        (adpcm_stream[3] << 24));
                    adpcm_stream += 4;

                    // loop over the 8 codes in this channels chunk.
                    // loop is structured like this to properly interleave the pcm data.
                    // otherwise we would have to store the decoded samples to several temp
                    // buffers and then interleave those temp buffers into the pcm stream.
                    for (std::size_t i = c; i < samples_this_chunk; i += channel_count) {
                        adpcm_states[c].code = codes & 0xF;
                        codes >>= 4;
                        adpcm_states[c] = decode_adpcm_sample(adpcm_states[c]);
                        pcm_stream[i] = adpcm_states[c].pcm_sample;
                    }
                }
                // skip over the chunk of samples we just decoded
                pcm_stream += samples_this_chunk;
                samples_remaining_this_block -= samples_this_chunk;
            }
        }
    }
}
