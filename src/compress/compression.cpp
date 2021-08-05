// SPDX-License-Identifier: GPL-3.0-only

#include <invader/compress/compression.hpp>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <cstdio>
#include <thread>
#include <filesystem>
#include <mutex>

#ifndef DISABLE_ZLIB
#include <zlib.h>
#endif

using namespace Invader::Parser;

namespace Invader::Compression {
    std::size_t compress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size, int compression_level) {
        const auto &header = *reinterpret_cast<const CacheFileHeader *>(data);
        auto &header_output = *reinterpret_cast<CacheFileHeader *>(output);
        
        if(!header.valid()) {
            throw InvalidMapException();
        }
        
        // If we're Xbox, we use a DEFLATE stream
        if(header.engine == CacheFileEngine::CACHE_FILE_XBOX) {
            #ifndef DISABLE_ZLIB
            auto input_padding_required = REQUIRED_PADDING_N_BYTES(data_size, CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE);
            if(input_padding_required) {
                eprintf_error("map size is not divisible by sector size (%zu)", static_cast<std::size_t>(CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE));
                throw CompressionFailureException();
            }

            // Compress that!
            z_stream deflate_stream = {};
            auto offset = sizeof(header);
            deflate_stream.zalloc = Z_NULL;
            deflate_stream.zfree = Z_NULL;
            deflate_stream.opaque = Z_NULL;
            deflate_stream.avail_in = data_size - offset;
            deflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(data + offset));
            deflate_stream.avail_out = output_size - offset;
            deflate_stream.next_out = reinterpret_cast<Bytef *>(output + offset);
            
            // Clamp
            if(compression_level > Z_BEST_COMPRESSION) {
                compression_level = Z_BEST_COMPRESSION;
            }
            else if(compression_level < Z_NO_COMPRESSION) {
                compression_level = Z_NO_COMPRESSION;
            }
            
            if((deflateInit(&deflate_stream, compression_level) != Z_OK) || (deflate(&deflate_stream, Z_FINISH) != Z_STREAM_END) || (deflateEnd(&deflate_stream) != Z_OK)) {
                throw DecompressionFailureException();
            }
            
            // Align to 4096 bytes
            header_output = header;
            std::size_t padding_required = REQUIRED_PADDING_N_BYTES(deflate_stream.total_out + sizeof(header), 4096);
            header_output.compressed_padding = static_cast<std::uint32_t>(padding_required);
            
            return deflate_stream.total_out + sizeof(header_output) + padding_required;
            
            #else
            std::terminate();
            #endif
        }
        
        // Otherwise, nope
        else {
            throw UnsupportedMapEngineException();
        }
    }

    std::size_t decompress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size) {
        // Check the header
        const auto &header = *reinterpret_cast<const CacheFileHeader *>(data);
        
        if(!header.valid()) {
            throw InvalidMapException();
        }
        
        if(header.engine == CacheFileEngine::CACHE_FILE_XBOX) {
            #ifndef DISABLE_ZLIB
            z_stream inflate_stream = {};
            inflate_stream.zalloc = Z_NULL;
            inflate_stream.zfree = Z_NULL;
            inflate_stream.opaque = Z_NULL;
            inflate_stream.avail_in = data_size - sizeof(header);
            inflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(data + sizeof(header)));
            inflate_stream.avail_out = output_size - sizeof(header);
            inflate_stream.next_out = reinterpret_cast<Bytef *>(output + sizeof(header));
            if((inflateInit(&inflate_stream) != Z_OK) || (inflate(&inflate_stream, Z_FINISH) != Z_STREAM_END) || (inflateEnd(&inflate_stream) != Z_OK)) {
                throw DecompressionFailureException();
            }
            return inflate_stream.total_out + sizeof(header);
            #else
            std::terminate();
            #endif
        }
        else {
            throw UnsupportedMapEngineException();
        }
    }

    std::vector<std::byte> compress_map_data(const std::byte *data, std::size_t data_size, int compression_level) {
        // Allocate the data
        const auto &header = *reinterpret_cast<const CacheFileHeader *>(data);
        std::vector<std::byte> new_data;
        if(data_size < sizeof(header)) {
            throw InvalidMapException();
        }
        
        // Allocate data
        new_data.resize(data_size * 2);

        // Compress
        auto compressed_size = compress_map_data(data, data_size, new_data.data(), new_data.size(), compression_level);

        // Resize and return it
        new_data.resize(compressed_size);

        return new_data;
    }

    std::vector<std::byte> decompress_map_data(const std::byte *data, std::size_t data_size) {
        // Allocate and decompress using data from the header
        const auto &header = *reinterpret_cast<const CacheFileHeader *>(data);
        if(!header.valid()) {
            throw InvalidMapException();
        }
        
        std::vector<std::byte> new_data;

        new_data.resize(header.decompressed_file_size);
        if(new_data.size() < sizeof(header)) {
            throw InvalidMapException();
        }
        
        // Decompress
        std::memcpy(new_data.data(), &header, sizeof(header));
        auto decompressed_size = decompress_map_data(data, data_size, new_data.data(), new_data.size());

        // Shrink the buffer to the new size
        new_data.resize(decompressed_size);

        return new_data;
    }
}
