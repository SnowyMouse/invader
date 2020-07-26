// SPDX-License-Identifier: GPL-3.0-only

#include <invader/compress/compression.hpp>
#include <invader/map/map.hpp>
#include <zstd.h>
#include <cstdio>
#include <filesystem>

#ifndef DISABLE_ZLIB
#include <zlib.h>
#endif

namespace Invader::Compression {
    template <typename T> static void compress_header(const Map &map, std::byte *header_output, std::size_t decompressed_size) {
        auto new_engine_version = map.get_engine();
        switch(new_engine_version) {
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                if(map.is_compressed()) {
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
        auto &header_out = *reinterpret_cast<T *>(header_output);
        header_out = {};
        header_out.crc32 = map.get_header_crc32();
        std::strncpy(header_out.build.string, map.get_build(), sizeof(header_out.build));
        std::strncpy(header_out.name.string, map.get_scenario_name(), sizeof(header_out.name));
        header_out.map_type = map.get_type();
        header_out.engine = new_engine_version;
        header_out.foot_literal = HEK::CacheFileLiteral::CACHE_FILE_FOOT;
        header_out.head_literal = HEK::CacheFileLiteral::CACHE_FILE_HEAD;
        header_out.tag_data_size = map.get_tag_data_length();
        header_out.tag_data_offset = map.get_tag_data_at_offset(0) - map.get_data_at_offset(0);
        if(new_engine_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            reinterpret_cast<HEK::NativeCacheFileHeader *>(&header_out)->compression_type = HEK::NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_ZSTD;
        }
        if(decompressed_size > UINT32_MAX) {
            throw MaximumFileSizeException();
        }
        header_out.decompressed_file_size = static_cast<std::uint32_t>(decompressed_size);
    }

    template <typename T> static void decompress_header(const std::byte *header_input, std::byte *header_output) {
        // Check to see if we can't even fit the header
        auto header_copy = *reinterpret_cast<const T *>(header_input);

        // Figure out the new engine version
        auto new_engine_version = header_copy.engine.read();
        bool invader_compression = false;
        bool stores_uncompressed_size;
        switch(header_copy.engine.read()) {
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                stores_uncompressed_size = false;
                throw MapNeedsCompressedException();
                break;
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                invader_compression = true;
                stores_uncompressed_size = true;
                if(reinterpret_cast<const HEK::NativeCacheFileHeader *>(header_input)->compression_type.read() == HEK::NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED) {
                    throw MapNeedsCompressedException();
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                stores_uncompressed_size = false;
                if(header_copy.decompressed_file_size.read() == 0) {
                    throw MapNeedsCompressedException();
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
                stores_uncompressed_size = false;
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                invader_compression = true;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
                stores_uncompressed_size = false;
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                invader_compression = true;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                stores_uncompressed_size = false;
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_DEMO;
                invader_compression = true;
                break;
            default:
                // Check if it's an uncompressed demo map?
                if(static_cast<HEK::CacheFileHeader>(*reinterpret_cast<const HEK::CacheFileDemoHeader *>(header_input)).engine.read() == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                    throw MapNeedsCompressedException();
                }

                // Give up
                throw UnsupportedMapEngineException();
        }

        // Determine if the file size isn't set correctly
        if((invader_compression && header_copy.decompressed_file_size < sizeof(header_copy)) || !header_copy.valid()) {
            throw InvalidMapException();
        }

        // Set the file size to either the original decompressed size or 0 (if needed) and the engine to the new thing
        header_copy.decompressed_file_size = stores_uncompressed_size ? header_copy.decompressed_file_size.read() : 0;
        header_copy.engine = new_engine_version;
        
        // Set the type, too, if need be
        if(new_engine_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            reinterpret_cast<HEK::NativeCacheFileHeader *>(&header_copy)->compression_type = HEK::NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED;
        }

        // if demo, convert the header, otherwise copy the header
        if(new_engine_version == HEK::CACHE_FILE_DEMO) {
            header_copy.foot_literal = HEK::CacheFileLiteral::CACHE_FILE_FOOT_DEMO;
            header_copy.head_literal = HEK::CacheFileLiteral::CACHE_FILE_HEAD_DEMO;
            *reinterpret_cast<HEK::CacheFileDemoHeader *>(header_output) = *reinterpret_cast<HEK::CacheFileHeader *>(&header_copy);
        }
        else {
            *reinterpret_cast<T *>(header_output) = header_copy;
        }
    }

    constexpr std::size_t HEADER_SIZE = sizeof(HEK::CacheFileHeader);

    std::size_t compress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size, int compression_level) {
        // Load the data
        auto map = Map::map_with_pointer(const_cast<std::byte *>(data), data_size);

        // If we're Xbox, we use a DEFLATE stream
        auto engine = map.get_engine();
        if(engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            #ifndef DISABLE_ZLIB
            compress_header<HEK::CacheFileHeader>(map, output, data_size);

            // Compress that!
            z_stream deflate_stream = {};
            auto offset = sizeof(HEK::CacheFileHeader);
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
            std::size_t padding_required = 4096 - ((deflate_stream.total_out + HEADER_SIZE) % 4096);
            if(padding_required) {
                reinterpret_cast<HEK::CacheFileHeader *>(output)->compressed_padding = static_cast<std::uint32_t>(padding_required);
            }
            
            return deflate_stream.total_out + HEADER_SIZE + padding_required;
            
            #else
            std::terminate();
            #endif
        }
        // Otherwise, we use zstandard
        else {
            if(engine == HEK::CACHE_FILE_NATIVE) {
                compress_header<HEK::NativeCacheFileHeader>(map, output, data_size);
            }
            else {
                compress_header<HEK::CacheFileHeader>(map, output, data_size);
            }

            // Immediately compress it
            auto compressed_size = ZSTD_compress(output + HEADER_SIZE, output_size - HEADER_SIZE, data + HEADER_SIZE, data_size - HEADER_SIZE, compression_level);
            if(ZSTD_isError(compressed_size)) {
                throw CompressionFailureException();
            }

            // Done
            return compressed_size + HEADER_SIZE;
        }
    }

    std::size_t decompress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size) {
        // Check the header
        const auto *header = reinterpret_cast<const HEK::CacheFileHeader *>(data);
        if(sizeof(*header) > data_size || !header->valid()) {
            auto demo_header = static_cast<const HEK::CacheFileHeader>(*reinterpret_cast<const HEK::CacheFileDemoHeader *>(header));
            if(demo_header.valid()) {
                if(demo_header.engine == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                    throw MapNeedsCompressedException();
                }
            }
            throw InvalidMapException();
        }

        if(header->engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            decompress_header<HEK::NativeCacheFileHeader>(data, output);
        }
        else {
            decompress_header<HEK::CacheFileHeader>(data, output);

            // If it's Xbox, do this
            if(header->engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
                #ifndef DISABLE_ZLIB
                z_stream inflate_stream = {};
                inflate_stream.zalloc = Z_NULL;
                inflate_stream.zfree = Z_NULL;
                inflate_stream.opaque = Z_NULL;
                inflate_stream.avail_in = data_size - sizeof(*header);
                inflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(data + sizeof(*header)));
                inflate_stream.avail_out = output_size - sizeof(*header);
                inflate_stream.next_out = reinterpret_cast<Bytef *>(output + sizeof(*header));
                if((inflateInit(&inflate_stream) != Z_OK) || (inflate(&inflate_stream, Z_FINISH) != Z_STREAM_END) || (inflateEnd(&inflate_stream) != Z_OK)) {
                    throw DecompressionFailureException();
                }
                return inflate_stream.total_out + HEADER_SIZE;
                #else
                std::terminate();
                #endif
            }
        }

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
        std::vector<std::byte> new_data;
        if(data_size < HEADER_SIZE) {
            throw InvalidMapException();
        }
        
        // Allocate data
        new_data.resize(ZSTD_compressBound(data_size - HEADER_SIZE) + HEADER_SIZE);

        // Compress
        auto compressed_size = compress_map_data(data, data_size, new_data.data(), new_data.size(), compression_level);

        // Resize and return it
        new_data.resize(compressed_size);

        return new_data;
    }

    std::vector<std::byte> decompress_map_data(const std::byte *data, std::size_t data_size) {
        // Allocate and decompress using data from the header
        const auto *header = reinterpret_cast<const HEK::CacheFileHeader *>(data);
        std::vector<std::byte> new_data = std::vector<std::byte>(header->decompressed_file_size);

        if(header->engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            decompress_header<HEK::NativeCacheFileHeader>(data, new_data.data());
        }
        else {
            decompress_header<HEK::CacheFileHeader>(data, new_data.data());
        }

        // Decompress
        auto decompressed_size = decompress_map_data(data, data_size, new_data.data(), new_data.size());

        // Shrink the buffer to the new size
        new_data.resize(decompressed_size);

        return new_data;
    }

    struct LowMemoryDecompression {
        /**
         * Callback for when a decompression occurs
         * @param decompressed_data decompressed data to write
         * @param size              size of decompressed data
         * @param user_data         user data to pass
         * @return                  true if successful
         */
        bool (*write_callback)(const std::byte *decompressed_data, std::size_t size, void *user_data) = nullptr;

        /**
         * Decompress the map file
         * @param path path to the map file
         */
         void decompress_map_file(const char *input, void *user_data) {
             // Open the input file
             std::FILE *input_file = std::fopen(input, "rb");
             if(!input_file) {
                 throw FailedToOpenFileException();
             }

             // Get the size
             std::size_t total_size = std::filesystem::file_size(input);

             // Read the input file header
             HEK::CacheFileHeader header_input;
             if(std::fread(&header_input, sizeof(header_input), 1, input_file) != 1) {
                 std::fclose(input_file);
                 throw DecompressionFailureException();
             }

             // Make the output header and write it
             std::byte header_output[HEADER_SIZE];
             try {
                 if(header_input.engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                     decompress_header<HEK::NativeCacheFileHeader>(reinterpret_cast<std::byte *>(&header_input), header_output);
                 }
                 else {
                     decompress_header<HEK::CacheFileHeader>(reinterpret_cast<std::byte *>(&header_input), header_output);
                 }
             }
             catch (std::exception &) {
                 std::fclose(input_file);
                 throw;
             }

             // Write the header
             if(!write_callback(header_output, sizeof(header_output), user_data)) {
                 std::fclose(input_file);
                 throw DecompressionFailureException();
             }

             // Allocate and init a stream
             auto decompression_stream = ZSTD_createDStream();
             const std::size_t init = ZSTD_initDStream(decompression_stream);

             std::size_t total_read = HEADER_SIZE;
             auto read_data = [&input_file, &decompression_stream, &total_read](std::byte *where, std::size_t size) {
                 if(std::fread(where, size, 1, input_file) != 1) {
                     std::fclose(input_file);
                     ZSTD_freeDStream(decompression_stream);
                     throw DecompressionFailureException();
                 }
                 total_read += size;
             };

             auto &write_callback = this->write_callback;
             auto write_data = [&input_file, &write_callback, &decompression_stream, &user_data](const std::byte *where, std::size_t size) {
                 if(!write_callback(where, size, user_data)) {
                     std::fclose(input_file);
                     ZSTD_freeDStream(decompression_stream);
                     throw DecompressionFailureException();
                 }
             };

             while(total_read < total_size) {
                 // Make some input/output data thingy
                 std::vector<std::byte> input_data(init);
                 std::vector<std::byte> output_data(ZSTD_DStreamOutSize());

                 // Read the first bit
                 read_data(input_data.data(), input_data.size());

                 for(;;) {
                     ZSTD_inBuffer_s input_buffer = {};
                     ZSTD_outBuffer_s output_buffer = {};
                     input_buffer.src = input_data.data();
                     input_buffer.size = input_data.size();
                     output_buffer.dst = output_data.data();
                     output_buffer.size = output_data.size();

                     // Get the output
                     std::size_t q = ZSTD_decompressStream(decompression_stream, &output_buffer, &input_buffer);
                     if(ZSTD_isError(q)) {
                         std::fclose(input_file);
                         ZSTD_freeDStream(decompression_stream);
                         throw DecompressionFailureException();
                     }

                     // Write it
                     if(output_buffer.pos) {
                         write_data(reinterpret_cast<std::byte *>(output_buffer.dst), output_buffer.pos);
                     }

                     // If it's > 0, we need more data
                     if(q > 0) {
                         input_data.clear();
                         input_data.insert(input_data.end(), q, std::byte());
                         read_data(input_data.data(), q);
                     }
                     else {
                         break;
                     }
                 }
             }

             // Close the stream and the files
             std::fclose(input_file);
             ZSTD_freeDStream(decompression_stream);
         }
    };

    std::size_t decompress_map_file(const char *input, const char *output) {
        struct OutputWriter {
            std::FILE *output_file;
            std::size_t output_position = 0;
        } output_writer = { std::fopen(output, "wb") };

        if(!output_writer.output_file) {
            std::fclose(output_writer.output_file);
            throw FailedToOpenFileException();
        }

        LowMemoryDecompression decomp;
        decomp.write_callback = [](const std::byte *decompressed_data, std::size_t size, void *user_data) -> bool {
            auto &output_writer = *reinterpret_cast<OutputWriter *>(user_data);
            output_writer.output_position += size;
            return std::fwrite(decompressed_data, size, 1, reinterpret_cast<std::FILE *>(output_writer.output_file));
        };

        try {
            decomp.decompress_map_file(input, &output_writer);
        }
        catch (std::exception &e) {
            std::fclose(output_writer.output_file);
            throw;
        }
        std::fclose(output_writer.output_file);

        return output_writer.output_position;
    }

    std::size_t decompress_map_file(const char *input, std::byte *output, std::size_t output_size) {
        struct OutputWriter {
            std::byte *output;
            std::size_t output_size;
            std::size_t output_position = 0;
        } output_writer = { output, output_size };

        LowMemoryDecompression decomp;
        decomp.write_callback = [](const std::byte *decompressed_data, std::size_t size, void *user_data) -> bool {
            OutputWriter &output_writer = *reinterpret_cast<OutputWriter *>(user_data);
            std::size_t new_position = output_writer.output_position + size;
            if(new_position > output_writer.output_size) {
                return false;
            }
            std::copy(decompressed_data, decompressed_data + size, output_writer.output + output_writer.output_position);
            output_writer.output_position = new_position;
            return true;
        };

        try {
            decomp.decompress_map_file(input, &output_writer);
        }
        catch (std::exception &e) {
            throw;
        }

        return output_writer.output_position;
    }
}
