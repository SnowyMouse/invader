// SPDX-License-Identifier: GPL-3.0-only

#include <invader/compress/compression.hpp>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <zstd.h>
#include <cstdio>
#include <thread>
#include <filesystem>
#include <mutex>

#ifndef DISABLE_ZLIB
#include <zlib.h>
#endif

namespace Invader::Compression {
    template <typename T> static void compress_header(const T &header_input, std::byte *header_output, std::size_t decompressed_size) {
        auto new_engine_version = header_input.engine;
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
                break;
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                if(reinterpret_cast<const HEK::NativeCacheFileHeader *>(&header_input)->compression_type.read() != HEK::NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED) {
                    throw MapNeedsCompressedException();
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
        header_out = header_input;
        header_out.engine = new_engine_version;
        header_out.foot_literal = HEK::CacheFileLiteral::CACHE_FILE_FOOT;
        header_out.head_literal = HEK::CacheFileLiteral::CACHE_FILE_HEAD;
        
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
        auto &header_input_s = *reinterpret_cast<const T *>(header_input);

        // Figure out the new engine version
        auto new_engine_version = header_input_s.engine.read();
        bool invader_compression = false;
        switch(header_input_s.engine.read()) {
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
            case HEK::CacheFileEngine::CACHE_FILE_DEMO:
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
                throw MapNeedsCompressedException();
                break;
            case HEK::CacheFileEngine::CACHE_FILE_NATIVE:
                invader_compression = true;
                if(reinterpret_cast<const HEK::NativeCacheFileHeader *>(&header_input_s)->compression_type.read() == HEK::NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED) {
                    throw MapNeedsCompressedException();
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                if(header_input_s.decompressed_file_size.read() == 0) {
                    throw MapNeedsCompressedException();
                }
                break;
            case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION_COMPRESSED:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                invader_compression = true;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_RETAIL_COMPRESSED:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                invader_compression = true;
                break;
            case HEK::CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED:
                new_engine_version = HEK::CacheFileEngine::CACHE_FILE_DEMO;
                invader_compression = true;
                break;
            default:
                // Check if it's an uncompressed demo map?
                if(static_cast<HEK::CacheFileHeader>(*reinterpret_cast<const HEK::CacheFileDemoHeader *>(&header_input_s)).engine.read() == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                    throw MapNeedsCompressedException();
                }

                // Give up
                throw UnsupportedMapEngineException();
        }

        // Determine if the file size isn't set correctly
        if((invader_compression && header_input_s.decompressed_file_size < sizeof(header_input_s)) || !header_input_s.valid()) {
            throw InvalidMapException();
        }
        
        // Set the type, too, if need be
        if(new_engine_version == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            auto &header_output_s = *reinterpret_cast<HEK::NativeCacheFileHeader *>(header_output);
            header_output_s = *reinterpret_cast<const HEK::NativeCacheFileHeader *>(header_input);
            reinterpret_cast<HEK::NativeCacheFileHeader *>(header_output)->compression_type = HEK::NativeCacheFileHeader::NativeCacheFileCompressionType::NATIVE_CACHE_FILE_COMPRESSION_UNCOMPRESSED;
        }
        else {
            auto &header_output_s = *reinterpret_cast<T *>(header_output);
            header_output_s = header_input_s;
            header_output_s.engine = new_engine_version;
            if(new_engine_version == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                header_output_s.foot_literal = HEK::CacheFileLiteral::CACHE_FILE_FOOT_DEMO;
                header_output_s.head_literal = HEK::CacheFileLiteral::CACHE_FILE_HEAD_DEMO;
                
                // Ensure it's the demo version
                if(!T::IS_DEMO) {
                    auto header_copy = *reinterpret_cast<HEK::CacheFileHeader *>(&header_output_s);
                    *reinterpret_cast<HEK::CacheFileDemoHeader *>(header_output) = header_copy;
                }
            }
        }
    }

    constexpr std::size_t HEADER_SIZE = sizeof(HEK::CacheFileHeader);

    std::size_t compress_map_data(const std::byte *data, std::size_t data_size, std::byte *output, std::size_t output_size, int compression_level) {
        const auto &header = *reinterpret_cast<const HEK::CacheFileHeader *>(data);
        const auto &header_demo = *reinterpret_cast<const HEK::CacheFileDemoHeader *>(data);
        const auto &header_native = *reinterpret_cast<const HEK::NativeCacheFileHeader *>(data);
        
        // If we're Xbox, we use a DEFLATE stream
        if(header.valid() && header.engine == HEK::CacheFileEngine::CACHE_FILE_XBOX) {
            #ifndef DISABLE_ZLIB
            auto input_padding_required = REQUIRED_PADDING_N_BYTES(data_size, HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE);
            if(input_padding_required) {
                eprintf_error("map size is not divisible by sector size (%zu)", static_cast<std::size_t>(HEK::CacheFileXboxConstants::CACHE_FILE_XBOX_SECTOR_SIZE));
                throw CompressionFailureException();
            }
            
            compress_header(header, output, data_size);

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
            std::size_t padding_required = REQUIRED_PADDING_N_BYTES(deflate_stream.total_out + HEADER_SIZE, 4096);
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
            if(header_native.valid() && header_native.engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
                compress_header(header_native, output, data_size);
                reinterpret_cast<HEK::NativeCacheFileHeader *>(output)->timestamp = reinterpret_cast<const HEK::NativeCacheFileHeader *>(data)->timestamp;
            }
            else if(header.valid()) {
                compress_header(header, output, data_size);
            }
            else if(header_demo.valid()) {
                compress_header(header_demo, output, data_size);
            }
            else {
                throw InvalidMapException();
            }

            // Immediately compress it
            auto *context = ZSTD_createCCtx();
            ZSTD_CCtx_setParameter(context, ZSTD_cParameter::ZSTD_c_enableLongDistanceMatching, 1);
            ZSTD_CCtx_setParameter(context, ZSTD_cParameter::ZSTD_c_compressionLevel, compression_level);
            auto compressed_size = ZSTD_compress2(context, output + HEADER_SIZE, output_size - HEADER_SIZE, data + HEADER_SIZE, data_size - HEADER_SIZE);
            ZSTD_freeCCtx(context);
            
            // Check if it's an error
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
            auto *demo_header = reinterpret_cast<const HEK::CacheFileDemoHeader *>(header);
            if(demo_header->valid()) {
                if(demo_header->engine == HEK::CacheFileEngine::CACHE_FILE_DEMO) {
                    throw MapNeedsCompressedException();
                }
                else if(demo_header->engine != HEK::CacheFileEngine::CACHE_FILE_DEMO_COMPRESSED) {
                    throw InvalidMapException();
                }
            }
        }
        
        if(header->valid() && header->engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            decompress_header<HEK::NativeCacheFileHeader>(data, output);
            reinterpret_cast<HEK::NativeCacheFileHeader *>(output)->timestamp = reinterpret_cast<const HEK::NativeCacheFileHeader *>(data)->timestamp;
        }
        else {
            if(header->valid()) {
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
            else if(reinterpret_cast<const HEK::CacheFileDemoHeader *>(data)->valid()) {
                decompress_header<HEK::CacheFileDemoHeader>(data, output);
            }
            else {
                throw InvalidMapException();
            }
        }

        // Immediately decompress
        auto decompressed_size = ZSTD_decompress(output + HEADER_SIZE, output_size - HEADER_SIZE, data + HEADER_SIZE, data_size - HEADER_SIZE);
        if(ZSTD_isError(decompressed_size) || (decompressed_size + HEADER_SIZE) != output_size) {
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
        const auto *demo_header = reinterpret_cast<const HEK::CacheFileDemoHeader *>(data);
        const auto *native_header = reinterpret_cast<const HEK::NativeCacheFileHeader *>(data);
        
        std::vector<std::byte> new_data;

        if(native_header->valid() && native_header->engine == HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            new_data.resize(native_header->decompressed_file_size);
            if(new_data.size() < sizeof(*native_header)) {
                throw InvalidMapException();
            }
            
            decompress_header<HEK::NativeCacheFileHeader>(data, new_data.data());
            reinterpret_cast<HEK::NativeCacheFileHeader *>(new_data.data())->timestamp = native_header->timestamp;
        }
        else if(header->valid()) {
            new_data.resize(header->decompressed_file_size);
            if(new_data.size() < sizeof(*header)) {
                throw InvalidMapException();
            }
            
            decompress_header<HEK::CacheFileHeader>(data, new_data.data());
        }
        else if(demo_header->valid()) {
            new_data.resize(demo_header->decompressed_file_size);
            if(new_data.size() < sizeof(*demo_header)) {
                throw InvalidMapException();
            }
            
            decompress_header<HEK::CacheFileDemoHeader>(data, new_data.data());
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
                     reinterpret_cast<HEK::NativeCacheFileHeader *>(header_output)->timestamp = reinterpret_cast<const HEK::NativeCacheFileHeader *>(&header_input)->timestamp;
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
    
    std::vector<std::byte> ceaflate_compress(const std::byte *input, std::size_t input_size, int compression_level) {
        #define MAXIMUM_CEAFLATE_CHUNK_SIZE 0x20000
        
        if(compression_level > Z_BEST_COMPRESSION) {
            compression_level = Z_BEST_COMPRESSION;
        }
        else if(compression_level < Z_NO_COMPRESSION) {
            compression_level = Z_NO_COMPRESSION;
        }
        
        // Set these up
        std::vector<std::vector<std::byte>> output_chunks;
        output_chunks.reserve((input_size + (MAXIMUM_CEAFLATE_CHUNK_SIZE - 1)) / MAXIMUM_CEAFLATE_CHUNK_SIZE);
        std::size_t current_offset = 0;
        std::mutex mutex;
        bool error = false;
        
        // Max threads?
        auto max_threads = std::thread::hardware_concurrency();
        if(max_threads < 1) {
            max_threads = 1;
        }
        
        auto compress_worker = [](std::mutex *mutex, std::vector<std::vector<std::byte>> *output_chunks, const std::byte *input, std::size_t input_size, std::size_t *current_offset, int compression_level, bool *error) {
            while(true) {
                mutex->lock();
                
                // Check if we're done
                if(*current_offset == input_size || *error) {
                    mutex->unlock();
                    return;
                }
                
                // Reserve one for us
                std::size_t chunk_index = output_chunks->size();
                output_chunks->emplace_back();
                
                // Get our input
                auto *input_data = input + *current_offset;
                
                // Get the chunk size
                std::size_t remaining_size = input_size - *current_offset;
                std::size_t chunk_size = remaining_size > MAXIMUM_CEAFLATE_CHUNK_SIZE ? MAXIMUM_CEAFLATE_CHUNK_SIZE : remaining_size;
                std::size_t compressed_chunk_size = MAXIMUM_CEAFLATE_CHUNK_SIZE * 4; // just in case
                (*current_offset) += chunk_size;
                
                // Done for now
                mutex->unlock();
                
                // Make our blob
                auto output_blob = std::make_unique<std::byte []>(sizeof(std::uint32_t) + compressed_chunk_size);
                auto &output_chunk_size = *reinterpret_cast<std::uint32_t *>(output_blob.get());
                output_chunk_size = chunk_size;
                auto *output_compressed_data = output_blob.get() + sizeof(output_chunk_size);
                
                // Compress
                z_stream deflate_stream = {};
                deflate_stream.zalloc = Z_NULL;
                deflate_stream.zfree = Z_NULL;
                deflate_stream.opaque = Z_NULL;
                deflate_stream.avail_in = chunk_size;
                deflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(input_data));
                deflate_stream.avail_out = compressed_chunk_size;
                deflate_stream.next_out = reinterpret_cast<Bytef *>(output_compressed_data);
                
                if((deflateInit(&deflate_stream, compression_level) != Z_OK) || (deflate(&deflate_stream, Z_FINISH) != Z_STREAM_END) || (deflateEnd(&deflate_stream) != Z_OK)) {
                    mutex->lock();
                    *error = true;
                    mutex->unlock();
                    return;
                }
                
                // Be done
                mutex->lock();
                (*output_chunks)[chunk_index] = std::vector<std::byte>(output_blob.get(), output_blob.get() + sizeof(output_chunk_size) + deflate_stream.total_out);
                mutex->unlock();
            }
        };
        
        // Create our threads
        std::vector<std::thread> threads;
        threads.reserve(max_threads);
        
        for(unsigned int i = 0; i < max_threads; i++) {
            threads.emplace_back(compress_worker, &mutex, &output_chunks, input, input_size, &current_offset, compression_level, &error);
        }
        
        // Wait for our threads to finish
        for(auto &i : threads) {
            i.join();
        }
        
        // Did it fail?
        if(error) {
            throw CompressionFailureException();
        }
        
        // No? Okay. We're almost done. Just gotta recombine everything
        std::size_t total_size = 0;
        for(auto &i : output_chunks) {
            total_size += i.size();
        }
        
        // Initialize with a buffer to hold our offsets
        std::size_t chunk_count = output_chunks.size();
        std::vector<std::byte> output(sizeof(std::uint32_t) * (chunk_count + 1));
        output.reserve(output.size() + total_size);
        *reinterpret_cast<std::uint32_t *>(output.data()) = static_cast<std::uint32_t>(chunk_count);
        
        // Concatenate
        for(std::size_t c = 0; c < chunk_count; c++) {
            reinterpret_cast<std::uint32_t *>(output.data())[c + 1] = static_cast<std::uint32_t>(output.size());
            auto &chunk = output_chunks[c];
            output.insert(output.end(), chunk.begin(), chunk.end());
        }
        
        // Done
        return output;
    }
    
    std::vector<std::byte> ceaflate_decompress(const std::byte *input, std::size_t input_size) {
        auto compression_size = ceaflate_compression_size(input, input_size);
        if(!compression_size.has_value()) {
            throw DecompressionFailureException();
        }
        
        // Allocate
        std::vector<std::byte> output(*compression_size);
        
        // Max threads?
        auto max_threads = std::thread::hardware_concurrency();
        if(max_threads < 1) {
            max_threads = 1;
        }
        
        // Begin
        std::vector<std::thread> threads;
        std::mutex mutex;
        std::size_t current_offset = 0;
        std::size_t current_chunk = 0;
        std::size_t total_written = 0;
        bool error = false;
        
        // Let's do it!
        auto decompress_worker = [](std::mutex *mutex, std::size_t *current_chunk, std::size_t *current_offset, std::size_t *total_written, const std::byte *input, std::size_t input_size, std::byte *write_to, bool *error) {
            // Get chunk information
            auto chunk_count = *reinterpret_cast<const std::uint32_t *>(input);
            const auto *offsets = reinterpret_cast<const std::uint32_t *>(input) + 1;
            
            while(true) {
                mutex->lock();
                
                // Are we done?
                if(*current_offset == input_size || *error) {
                    mutex->unlock();
                    return;
                }
                
                // Get our chunk
                const std::byte *chunk_start = input + offsets[*current_chunk];
                auto uncompressed_chunk_size = *reinterpret_cast<const std::uint32_t *>(chunk_start);
                chunk_start += sizeof(uncompressed_chunk_size);
                
                // Increment chunk count
                (*current_chunk)++;
                
                // Get the end
                const std::byte *chunk_end = input + input_size;
                if(*current_chunk < chunk_count) { // if we aren't at the end, get the offset of the next chunk
                    chunk_end = input + offsets[*current_chunk];
                }
                (*current_offset) = chunk_end - input;
                
                // Where are we writing to?
                std::byte *write_to_this_chunk = write_to + *total_written;
                
                // And of course, increment this
                (*total_written) += uncompressed_chunk_size;
                
                // We no longer need this locked
                mutex->unlock();
                
                // All right. Here's the size of the compressed data
                std::size_t compressed_chunk_size = chunk_end - chunk_start;
                
                // Do it!
                z_stream inflate_stream = {};
                inflate_stream.zalloc = Z_NULL;
                inflate_stream.zfree = Z_NULL;
                inflate_stream.opaque = Z_NULL;
                inflate_stream.avail_in = compressed_chunk_size;
                inflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(chunk_start));
                inflate_stream.avail_out = uncompressed_chunk_size;
                inflate_stream.next_out = reinterpret_cast<Bytef *>(write_to_this_chunk);
                if((inflateInit(&inflate_stream) != Z_OK) || (inflate(&inflate_stream, Z_FINISH) != Z_STREAM_END) || (inflateEnd(&inflate_stream) != Z_OK)) {
                    mutex->lock();
                    *error = true;
                    mutex->unlock();
                    return;
                }
            }
        };
        
        // Create our threads
        threads.reserve(max_threads);
        for(unsigned int i = 0; i < max_threads; i++) {
            threads.emplace_back(decompress_worker, &mutex, &current_chunk, &current_offset, &total_written, input, input_size, output.data(), &error);
        }
        
        // Wait for our threads to finish
        for(auto &i : threads) {
            i.join();
        }
        
        // Did it fail?
        if(error) {
            throw DecompressionFailureException();
        }
        
        // No? Okay. We're done!
        return output;
    }
    
    std::optional<std::size_t> ceaflate_compression_size(const std::byte *input, std::size_t input_size) noexcept {
        // Can we hold the count?
        if(input_size < sizeof(std::uint32_t)) {
            return std::nullopt;
        }
        
        // Can we hold the count plus the number of offsets?
        const auto &count = *reinterpret_cast<const std::uint32_t *>(input);
        if(input_size < (count + 1) * sizeof(std::uint32_t)) {
            return std::nullopt;
        }
        
        // Let's begin
        std::uint64_t total_size = 0;
        
        std::uint64_t current_offset = 0;
        const auto *offsets = &count + 1;
        
        for(std::size_t i = 0; i < count; i++) {
            std::uint64_t new_offset = offsets[i];
            
            // Can we hold it?
            if(new_offset + sizeof(std::uint32_t) > input_size) {
                return std::nullopt;
            }
            
            // Check to make sure we can hold a 32-bit integer, and also ensure the offset is greater than the previous
            if(new_offset < current_offset + sizeof(std::uint32_t)) {
                return std::nullopt;
            }
            
            // Add to our size
            total_size += *reinterpret_cast<const std::uint32_t *>(input + new_offset);
            
            // Set the current offset to here
            current_offset = new_offset;
        }
        
        return total_size;
    }
}
