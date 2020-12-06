// SPDX-License-Identifier: GPL-3.0-only

#include <invader/printf.hpp>
#include <invader/error.hpp>
#include <invader/sound/sound_reader.hpp>
#include <FLAC/stream_decoder.h>
#include <memory>

namespace Invader::SoundReader {
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

    Sound sound_from_flac_file(const std::filesystem::path &path) {
        Sound result = {};
        auto path_str = path.string();

        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
        try {
            if(FLAC__stream_decoder_init_file(decoder, path_str.c_str(), write_flac_data, on_flac_metadata, on_flac_error, &result) != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
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
        stream_holder_stuff.offset += bytes_to_store;

        // And then end here
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    static FLAC__StreamDecoderSeekStatus seek_flac_data(const FLAC__StreamDecoder *, FLAC__uint64 absolute_byte_offset, void *client_data) noexcept {
        auto &client_data_sound = *reinterpret_cast<SoundReader::Sound *>(client_data);
        auto &stream_holder_stuff = *reinterpret_cast<StreamHolder *>(client_data_sound.internal);
        if(absolute_byte_offset > stream_holder_stuff.data_length) {
            stream_holder_stuff.offset = stream_holder_stuff.data_length;
        }
        else {
            stream_holder_stuff.offset = absolute_byte_offset;
        }
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
                eprintf_error("Failed to process FLAC stream");
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
