// SPDX-License-Identifier: GPL-3.0-only

#include <invader/compress/compression.hpp>
#include <invader/map/map.hpp>
#include <zstd.h>

namespace Invader::Compression {
    static void compress_header(const std::byte *header_input, std::byte *header_output, std::size_t decompressed_size) {
        // Check the header
        const auto &header = *reinterpret_cast<const HEK::CacheFileHeader *>(header_input);
        if(!header.valid()) {
            throw InvalidMapException();
        }

        auto new_engine_version = header.engine.read();
        switch(header.engine.read()) {
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET:
                if(header.decompressed_file_size.read() > 0) {
                    throw MapNeedsDecompressedException();
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
            case HEK::CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                throw MapNeedsDecompressedException();
            default:
                throw UnsupportedMapEngineException();
        }

        // Write the header
        auto &header_out = *reinterpret_cast<HEK::CacheFileHeader *>(header_output);
        header_out = header;
        header_out.engine = new_engine_version;
        header_out.foot_literal = HEK::CacheFileLiteral::CACHE_FILE_FOOT;
        header_out.head_literal = HEK::CacheFileLiteral::CACHE_FILE_HEAD;
        if(decompressed_size > UINT32_MAX) {
            throw MaximumFileSizeException();
        }
        header_out.decompressed_file_size = static_cast<std::uint32_t>(decompressed_size);
    }

    static void decompress_header(const std::byte *header_input, std::byte *header_output) {
        // Check to see if we can't even fit the header
        auto header_copy = *reinterpret_cast<const HEK::CacheFileHeader *>(header_input);
        if(header_copy.decompressed_file_size < sizeof(header_copy) || !header_copy.valid()) {
            throw InvalidMapException();
        }

        // Figure out the new engine version
        auto new_engine_version = header_copy.engine.read();
        switch(header_copy.engine.read()) {
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                throw MapNeedsCompressedException();
                break;
            case HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET:
                if(header_copy.decompressed_file_size.read() == 0) {
                    throw MapNeedsCompressedException();
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_DEMO;
                break;
            default:
                throw UnsupportedMapEngineException();
        }

        // Set the file size to 0 and the engine to the new thing
        header_copy.decompressed_file_size = 0;
        header_copy.engine = new_engine_version;

        // if demo, convert the header, otherwise copy the header
        if(new_engine_version == HEK::CACHE_FILE_DEMO) {
            header_copy.foot_literal = HEK::CacheFileLiteral::CACHE_FILE_FOOT_DEMO;
            header_copy.head_literal = HEK::CacheFileLiteral::CACHE_FILE_HEAD_DEMO;
            *reinterpret_cast<HEK::CacheFileDemoHeader *>(header_output) = header_copy;
        }
        else {
            *reinterpret_cast<HEK::CacheFileHeader *>(header_output) = header_copy;
        }
    }

    constexpr std::size_t HEADER_SIZE = sizeof(HEK::CacheFileHeader);

    std::size_t compress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size, int compression_level) {
        // Load the data
        auto map = Map::map_with_pointer(const_cast<std::byte *>(data), data_size);

        // Allocate the data
        compress_header(reinterpret_cast<const std::byte *>(&map.get_cache_file_header()), output, data_size);

        // Immediately compress it
        auto compressed_size = ZSTD_compress(output + HEADER_SIZE, output_size - HEADER_SIZE, data + HEADER_SIZE, data_size - HEADER_SIZE, compression_level);
        if(ZSTD_isError(compressed_size)) {
            throw CompressionFailureException();
        }

        // Done
        return compressed_size + HEADER_SIZE;
    }

    std::size_t decompress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size) {
        // Check the header
        const auto *header = reinterpret_cast<const HEK::CacheFileHeader *>(data);
        if(sizeof(*header) > data_size || !header->valid()) {
            auto demo_header = static_cast<const HEK::CacheFileHeader>(*reinterpret_cast<const HEK::CacheFileDemoHeader *>(header));
            if(demo_header.valid() && demo_header.engine == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                throw MapNeedsCompressedException();
            }
            throw InvalidMapException();
        }

        decompress_header(data, output);

        // Immediately decompress
        auto decompressed_size = ZSTD_decompress(output + HEADER_SIZE, output_size - HEADER_SIZE, data + HEADER_SIZE, data_size - HEADER_SIZE);
        if(ZSTD_isError(decompressed_size) || (decompressed_size + HEADER_SIZE) != header->decompressed_file_size) {
            throw DecompressionFailureException();
        }

        // Done
        return decompressed_size + HEADER_SIZE;
    }

    std::vector<std::byte> compress_map_data(const std::byte *data, std::size_t data_size, int compression_level) {
        // Allocate the data
        std::vector<std::byte> new_data(ZSTD_compressBound(data_size - HEADER_SIZE) + HEADER_SIZE);

        // Compress
        auto compressed_size = compress_map_data(data, data_size, new_data.data(), new_data.size(), compression_level);

        // Resize and return it
        new_data.resize(compressed_size);

        return new_data;
    }

    std::vector<std::byte> decompress_map_data(const std::byte *data, std::size_t data_size) {
        // Allocate and decompress using data from the header
        const auto *header = reinterpret_cast<const HEK::CacheFileHeader *>(data);
        std::vector<std::byte> new_data(header->decompressed_file_size);
        decompress_header(data, new_data.data());

        // Compress
        auto decompressed_size = decompress_map_data(data, data_size, new_data.data(), new_data.size());

        // Shrink the buffer to the new size
        new_data.resize(decompressed_size);

        return new_data;
    }
}
