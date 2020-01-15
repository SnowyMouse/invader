// SPDX-License-Identifier: GPL-3.0-only

#include <invader/hek/endian.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/sound/sound_reader.hpp>
#include <invader/sound/sound_encoder.hpp>
#include <FLAC/stream_decoder.h>
#include <climits>
#include <memory>

namespace Invader::SoundReader {
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

    Sound sound_from_wav_file(const char *path) {
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
            std::size_t fmt_subchunk_size = fmt_subchunk.subchunk_size;
            std::size_t expected_fmt_subchunk_size = sizeof(WAVFmtSubchunk) - sizeof(WAVSubchunkHeader);
            if(fmt_subchunk_size < expected_fmt_subchunk_size) {
                eprintf_error("Fmt subchunk size is wrong");
                throw InvalidInputSoundException();
            }

            // Handle WAV files that are too big
            std::size_t excess_data_ignored = fmt_subchunk_size - expected_fmt_subchunk_size;
            while(excess_data_ignored) {
                long data_we_can_ignore = excess_data_ignored > LONG_MAX ? LONG_MAX : static_cast<long>(excess_data_ignored);
                std::fseek(file, data_we_can_ignore, SEEK_CUR);
                excess_data_ignored -= data_we_can_ignore;
            }

            // Make sure it's something we can handle
            if(fmt_subchunk.audio_format != 1 && fmt_subchunk.audio_format != 3) {
                eprintf_error("WAV data type (%u) is not integer or floating point PCM", static_cast<unsigned int>(fmt_subchunk.audio_format));
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

            // Search for the data subchunk
            WAVSubchunkHeader subchunk = {};
            while(true) {
                READ_OR_BAIL(subchunk);
                if(subchunk.subchunk_id == 0x64617461) {
                    break;
                }
                else {
                    std::fseek(file, static_cast<long>(subchunk.subchunk_size.read()), SEEK_CUR);
                }
            }

            // Convert PCM to integer
            std::size_t data_size = subchunk.subchunk_size;
            if(fmt_subchunk.audio_format == 1) {
                result.pcm = std::vector<std::byte>(data_size);
                if(std::fread(result.pcm.data(), data_size, 1, file) != 1) {
                    eprintf_error("Failed to read data");
                    throw InvalidInputSoundException();
                }
            }
            else if(fmt_subchunk.audio_format == 3) {
                std::vector<float> pcm_float(data_size / sizeof(*pcm_float.data()));
                if(std::fread(pcm_float.data(), data_size, 1, file) != 1) {
                    eprintf_error("Failed to read data");
                    throw InvalidInputSoundException();
                }
                result.bits_per_sample = 24;
                result.pcm = SoundEncoder::convert_float_to_int(pcm_float, 24);
            }
            else {
                std::terminate();
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

    static FLAC__StreamDecoderWriteStatus write_flac_data(const FLAC__StreamDecoder *, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data) noexcept {
        auto &result = *reinterpret_cast<SoundReader::Sound *>(client_data);
        auto bytes = frame->header.bits_per_sample / 8;
        for(std::size_t i = 0; i < frame->header.blocksize; i++) {
            for(std::size_t c = 0; c < frame->header.channels; c++) {
                auto &s = buffer[c][i];
                for(std::size_t b = 0; b < bytes; b++) {
                    result.pcm.emplace_back(static_cast<std::byte>((s >> b * 8) & 0xFF));
                }
            }
        }
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }

    static void on_flac_metadata(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *client_data) noexcept {
        if(metadata->type == FLAC__MetadataType::FLAC__METADATA_TYPE_STREAMINFO) {
            auto &result = *reinterpret_cast<SoundReader::Sound *>(client_data);
            auto &stream_info = metadata->data.stream_info;
            result.bits_per_sample = stream_info.bits_per_sample;
            result.channel_count = stream_info.channels;
            result.sample_rate = stream_info.sample_rate;
            result.input_bits_per_sample = stream_info.bits_per_sample;
            result.input_channel_count = stream_info.channels;
            result.input_sample_rate = stream_info.sample_rate;
        }
    }

    static void on_flac_error(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus, void *client_data) noexcept {
        reinterpret_cast<SoundReader::Sound *>(client_data)->pcm.clear();
    }

    Sound sound_from_flac_file(const char *path) {
        Sound result = {};

        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
        try {
            if(FLAC__stream_decoder_init_file(decoder, path, write_flac_data, on_flac_metadata, on_flac_error, &result) != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
                eprintf_error("Failed to init FLAC stream");
                throw InvalidInputSoundException();
            }
            if(!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
                eprintf_error("Failed to init FLAC stream");
                throw InvalidInputSoundException();
            }
            if(result.pcm.size() == 0) {
                eprintf_error("Invalid or empty PCM stream from FLAC");
                throw InvalidInputSoundException();
            }
            FLAC__stream_decoder_delete(decoder);
        }
        catch(std::exception &) {
            FLAC__stream_decoder_delete(decoder);
            throw;
        }

        return result;
    }

    struct StreamHolder {
        const std::byte *data;
        std::size_t data_length;
        std::size_t offset;
    };

    static FLAC__StreamDecoderReadStatus read_flac_data(const FLAC__StreamDecoder *, FLAC__byte buffer[], std::size_t *bytes, void *client_data) noexcept {
        auto &client_data_sound = *reinterpret_cast<SoundReader::Sound *>(client_data);
        auto &stream_holder_stuff = *reinterpret_cast<StreamHolder *>(client_data_sound.internal);
        std::size_t data_remaining = stream_holder_stuff.data_length - stream_holder_stuff.offset;

        // If we're at the end of the stream, return here
        if(data_remaining == 0) {
            *bytes = 0;
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }

        // Otherwise, read it
        std::size_t bytes_to_store = data_remaining > *bytes ? *bytes : data_remaining;
        *bytes = bytes_to_store;
        std::memcpy(buffer, stream_holder_stuff.data + stream_holder_stuff.offset, bytes_to_store);
        stream_holder_stuff.offset += data_remaining;

        // And then end here
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    static FLAC__StreamDecoderSeekStatus seek_flac_data(const FLAC__StreamDecoder *, FLAC__uint64 absolute_byte_offset, void *client_data) noexcept {
        auto &client_data_sound = *reinterpret_cast<SoundReader::Sound *>(client_data);
        auto &stream_holder_stuff = *reinterpret_cast<StreamHolder *>(client_data_sound.internal);
        std::size_t new_offset = stream_holder_stuff.offset + absolute_byte_offset;
        if(new_offset < stream_holder_stuff.offset || new_offset > stream_holder_stuff.data_length) {
            return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
        }
        stream_holder_stuff.offset = new_offset;
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }

    static FLAC__StreamDecoderTellStatus tell_flac_data(const FLAC__StreamDecoder *, FLAC__uint64 *absolute_byte_offset, void *client_data) noexcept {
        auto &client_data_sound = *reinterpret_cast<SoundReader::Sound *>(client_data);
        auto &stream_holder_stuff = *reinterpret_cast<StreamHolder *>(client_data_sound.internal);
        *absolute_byte_offset = static_cast<FLAC__uint64>(stream_holder_stuff.offset);
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }

    static FLAC__StreamDecoderLengthStatus length_flac_data(const FLAC__StreamDecoder *, FLAC__uint64 *stream_length, void *client_data) noexcept {
        auto &client_data_sound = *reinterpret_cast<SoundReader::Sound *>(client_data);
        auto &stream_holder_stuff = *reinterpret_cast<StreamHolder *>(client_data_sound.internal);
        *stream_length = static_cast<FLAC__uint64>(stream_holder_stuff.data_length);
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }

    static FLAC__bool eof_flac_data(const FLAC__StreamDecoder *, void *client_data) noexcept {
        auto &client_data_sound = *reinterpret_cast<SoundReader::Sound *>(client_data);
        auto &stream_holder_stuff = *reinterpret_cast<StreamHolder *>(client_data_sound.internal);
        return stream_holder_stuff.offset == stream_holder_stuff.data_length;
    }

    Sound sound_from_flac(const std::byte *data, std::size_t data_length) {
        Sound result = {};

        // Initialize the data holder
        auto data_holder = std::make_unique<StreamHolder>();
        result.internal = data_holder.get();
        data_holder->data = data;
        data_holder->data_length = data_length;
        data_holder->offset = 0;

        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
        try {
            if(FLAC__stream_decoder_init_stream(decoder, read_flac_data, seek_flac_data, tell_flac_data, length_flac_data, eof_flac_data, write_flac_data, on_flac_metadata, on_flac_error, &result) != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
                eprintf_error("Failed to init FLAC stream");
                throw InvalidInputSoundException();
            }
            if(!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
                eprintf_error("Failed to init FLAC stream");
                throw InvalidInputSoundException();
            }
            if(result.pcm.size() == 0) {
                eprintf_error("Invalid or empty PCM stream from FLAC");
                throw InvalidInputSoundException();
            }
            FLAC__stream_decoder_delete(decoder);
        }
        catch(std::exception &) {
            FLAC__stream_decoder_delete(decoder);
            throw;
        }
        return result;
    }
}
