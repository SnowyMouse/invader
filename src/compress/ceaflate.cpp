// SPDX-License-Identifier: GPL-3.0-only

#ifndef DISABLE_ZLIB

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <mutex>
#include <climits>
#include <thread>
#include <zlib.h>
#include <invader/printf.hpp>

#else

#include <exception>

#endif

#include <invader/compress/ceaflate.hpp>

namespace Invader::Compression::Ceaflate {
    #ifndef DISABLE_ZLIB
    
    #define CHUNK_SIZE static_cast<std::size_t>(0x20000)

    struct CompressedMapHeader {
        static const constexpr std::size_t MAX_BLOCKS = 0xFFFF;
        std::uint32_t block_count;
        std::uint32_t block_offsets[MAX_BLOCKS];
    };

    struct Worker {
        const std::byte *input = nullptr;
        std::size_t input_size = 0;
        std::byte *output;
        std::size_t output_size = 0;
        std::size_t offset = 0;
        bool failure = false;
        std::mutex mutex;
        bool started = false;
    };

    static void perform_decompression(Worker *worker);
    static void perform_compression(Worker *worker);
    static void perform_job(std::vector<Worker> &workers, void (*function)(Worker *));

    std::size_t find_decompressed_file_size(const std::byte *input_data, std::size_t input_size) noexcept {
        // Read the file
        const auto &header = *reinterpret_cast<const CompressedMapHeader *>(input_data);

        // Make sure it's not bullshit
        auto block_count = static_cast<std::size_t>(header.block_count);
        if(header.block_count > CompressedMapHeader::MAX_BLOCKS) {
            return 0;
        }
        else if(header.block_count == 0) {
            return 0;
        }

        // Find the size
        std::size_t output_offset = 0;
        for(std::size_t i = 0; i < block_count; i++) {
            std::size_t offset = header.block_offsets[i];
            if(offset + sizeof(std::uint32_t) > input_size) {
                return 0;
            }

            const auto *input = input_data + offset;
            auto &uncompressed_size = *reinterpret_cast<const std::uint32_t *>(input);
            output_offset += uncompressed_size;
        }

        return output_offset;
    }

    bool decompress_file(const std::byte *input_data, std::size_t input_size, std::byte *output_data, std::size_t &output_size) {
        // Get the real size
        auto real_decompressed_size = find_decompressed_file_size(input_data, input_size);
        if(real_decompressed_size == 0) {
            eprintf_error("Compressed data appears to be invalid");
            return false;
        }
        if(real_decompressed_size > output_size) {
            eprintf_error("Not enough space in buffer to decompress (%zu > %zu)", real_decompressed_size, real_decompressed_size);
            return false;
        }

        // Read the file
        const auto &header = *reinterpret_cast<const CompressedMapHeader *>(input_data);
        auto block_count = static_cast<std::size_t>(header.block_count);

        // Allocate workers
        std::vector<Worker> workers(block_count);
        std::size_t output_offset = 0;
        for(std::size_t i = 0; i < block_count; i++) {
            // Set up the worker
            std::size_t offset = header.block_offsets[i];
            const auto *input = input_data + offset;
            auto &uncompressed_size = *reinterpret_cast<const std::uint32_t *>(input);

            auto remaining_size = input_size - (offset + sizeof(uncompressed_size));
            workers[i].input = input + sizeof(uncompressed_size);
            workers[i].input_size = remaining_size;
            workers[i].output = output_data + output_offset;
            workers[i].output_size = uncompressed_size;

            output_offset += uncompressed_size;
        }

        // Do it!
        perform_job(workers, perform_decompression);

        // If we failed, error out
        for(std::size_t i = 0; i < block_count; i++) {
            if(workers[i].failure) {
                eprintf_error("One or more blocks failed to decompress");
                return false;
            }
        }

        return true;
    }

    #define CHUNK_SIZE static_cast<std::size_t>(0x20000)

    struct CacheFileHeader {
        std::uint32_t head_literal;
        std::uint32_t engine;
        std::uint32_t decompressed_file_size;
        std::uint8_t pad1[0x4];
        std::uint32_t tag_data_offset;
        std::uint32_t tag_data_size;
        std::uint8_t pad2[0x8];
        char name[0x20];
        char build[0x20];
        std::uint16_t map_type;
        std::byte pad3[0x2];
        std::uint32_t crc32;
        std::byte pad4[0x794];
        std::uint32_t foot_literal;
    };
    static_assert(sizeof(CacheFileHeader) == 0x800);

    bool compress_file(const std::byte *input_data, std::size_t input_size, std::byte *output_data, std::size_t &output_size) {
        // Read the file
        std::size_t block_count = input_size / CHUNK_SIZE + ((input_size % CHUNK_SIZE) > 0);

        // Make a header
        CompressedMapHeader &header = *reinterpret_cast<CompressedMapHeader *>(output_data);
        if(block_count > CompressedMapHeader::MAX_BLOCKS) {
            eprintf_error("Maximum blocks exceeded (#%zu > %zu)", block_count, CompressedMapHeader::MAX_BLOCKS);
            return false;
        }
        header.block_count = static_cast<std::uint32_t>(block_count);

        // Allocate workers
        std::vector<Worker> workers(block_count);
        std::vector<std::unique_ptr<std::byte []>> workers_data(block_count);

        for(std::size_t i = 0; i < block_count; i++) {
            std::size_t offset = i * CHUNK_SIZE;

            // Set up the worker
            auto *input = input_data + offset;
            std::size_t remaining_size = input_size - offset;
            if(remaining_size > CHUNK_SIZE) {
                remaining_size = CHUNK_SIZE;
            }
            workers[i].offset = sizeof(std::uint32_t);
            std::size_t max_size = remaining_size * 2 + workers[i].offset;

            workers[i].input = input;
            workers[i].input_size = remaining_size;
            workers_data[i] = std::make_unique<std::byte []>(max_size);
            workers[i].output = workers_data[i].get();
            workers[i].output_size = max_size;
        }

        // Do it!
        perform_job(workers, perform_compression);

        // If we failed, error out
        bool failed = false;
        for(std::size_t i = 0; i < block_count; i++) {
            if(workers[i].failure) {
                eprintf_error("Block #%zu failed to compress", i);
                failed = true;
            }
        }
        if(failed) {
            return false;
        }

        // Set the offsets
        std::size_t current_offset = sizeof(header);
        for(std::size_t i = 0; i < block_count; i++) {
            if(current_offset > UINT32_MAX) {
                eprintf_error("Size exceeds the maximum limit (%zu > %zu)", current_offset, static_cast<std::size_t>(UINT32_MAX));
                return false;
            }

            if(output_size - current_offset < workers[i].output_size) {
                eprintf_error("Not enough space in buffer to compress");
                return false;
            }

            header.block_offsets[i] = static_cast<std::uint32_t>(current_offset);
            std::copy(workers[i].output, workers[i].output + workers[i].output_size, output_data + current_offset);
            current_offset += workers[i].output_size;
        }

        output_size = current_offset;
        return true;
    }

    static void perform_decompression(Worker *worker) {
        z_stream inflate_stream = {};
        inflate_stream.zalloc = Z_NULL;
        inflate_stream.zfree = Z_NULL;
        inflate_stream.opaque = Z_NULL;
        inflate_stream.avail_in = worker->input_size;
        inflate_stream.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(worker->input));
        inflate_stream.avail_out = worker->output_size;
        inflate_stream.next_out = reinterpret_cast<Bytef *>(worker->output);
        if((inflateInit(&inflate_stream) != Z_OK) || (inflate(&inflate_stream, Z_FINISH) != Z_STREAM_END) || (inflateEnd(&inflate_stream) != Z_OK)) {
            worker->failure = true;
        }
        worker->mutex.unlock();
    }

    static void perform_compression(Worker *worker) {
        z_stream deflate_stream = {};
        deflate_stream.zalloc = Z_NULL;
        deflate_stream.zfree = Z_NULL;
        deflate_stream.opaque = Z_NULL;
        deflate_stream.avail_in = worker->input_size;
        deflate_stream.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(worker->input));
        deflate_stream.avail_out = worker->output_size - sizeof(std::uint32_t);
        deflate_stream.next_out = reinterpret_cast<Bytef *>(worker->output) + sizeof(std::uint32_t);
        *reinterpret_cast<std::uint32_t *>(worker->output) = static_cast<std::uint32_t>(worker->input_size);
        if((deflateInit(&deflate_stream, Z_BEST_COMPRESSION) != Z_OK) || (deflate(&deflate_stream, Z_FINISH) != Z_STREAM_END) || (deflateEnd(&deflate_stream) != Z_OK)) {
            worker->failure = true;
        }
        else {
            worker->output_size = deflate_stream.total_out + sizeof(std::uint32_t);
        }
        worker->mutex.unlock();
    }

    static std::size_t busy_workers(std::vector<Worker> &workers);
    static Worker *next_worker(std::vector<Worker> &workers);

    static void perform_job(std::vector<Worker> &workers, void (*function)(Worker *)) {
        if(workers.size() == 0) {
            return;
        }

        // Lock the mutex!
        for(auto &w : workers) {
            w.mutex.lock();
        }

        // If we aren't threaded, just do a for loop
        std::size_t max_threads = std::thread::hardware_concurrency();
        if(max_threads <= 1) {
            for(auto &worker : workers) {
                function(&worker);
            }
        }

        // Otherwise, do it!
        else {
            while(true) {
                auto *next = next_worker(workers);
                auto busy_count = busy_workers(workers);
                if(busy_count == 0 && !next) {
                    break;
                }
                else if(next && busy_count < max_threads) {
                    next->started = true;
                    std::thread(function, next).detach();
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
    }

    static std::size_t busy_workers(std::vector<Worker> &workers) {
        std::size_t count = 0;
        for(auto &w : workers) {
            if(w.mutex.try_lock()) {
                w.mutex.unlock();
            }
            else {
                count += w.started;
            }
        }
        return count;
    }

    static Worker *next_worker(std::vector<Worker> &workers) {
        for(auto &w : workers) {
            if(!w.started) {
                return &w;
            }
        }
        return nullptr;
    }

    #else

    std::size_t find_decompressed_file_size(const std::byte *, std::size_t) noexcept {
        std::terminate();
    }

    bool decompress_file(const std::byte *, std::size_t , std::byte *, std::size_t &) {
        std::terminate();
    }

    bool compress_file(const std::byte *, std::size_t , std::byte *, std::size_t &) {
        std::terminate();
    }

    #endif
}
