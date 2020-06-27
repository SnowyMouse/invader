// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <zstd.h>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/compress/compression.hpp>

#define COMPRESSION_FORMAT_DEFLATE "deflate"
#define COMPRESSION_FORMAT_ZSTANDARD "zstandard"

int main(int argc, const char **argv) {
    using namespace Invader;

    struct CompressOptions {
        const char *output = nullptr;
        long compression_level = 19;
        bool decompress = false;
    } compress_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("output", 'o', 1, "Emit the resulting map at the given path. By default, this is the map path (overwrite).", "<file>");
    options.emplace_back("level", 'l', 1, "Set the compression level. Must be between 1 and 19. If compressing an Xbox map, this will be clamped from 1 to 9. Default: 19", "<level>");
    options.emplace_back("decompress", 'd', 0, "Decompress instead of compress.");

    static constexpr char DESCRIPTION[] = "Compress cache files.";
    static constexpr char USAGE[] = "[options] <map>";

    auto remaining_arguments = CommandLineOption::parse_arguments<CompressOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, compress_options, [](char opt, const auto &arguments, auto &compress_options) {
        switch(opt) {
            case 'd':
                compress_options.decompress = true;
                break;
            case 'l':
                compress_options.compression_level = std::strtol(arguments[0], nullptr, 10);
                if(compress_options.compression_level < 1 || compress_options.compression_level > 19) {
                    eprintf_error("Compression level must be between 1 and 19");
                    std::exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                compress_options.output = arguments[0];
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                break;
        }
    });

    // Get the input (and maybe output?)
    const char *input = remaining_arguments[0];
    if(compress_options.output == nullptr) {
        compress_options.output = input;
    }

    auto start = std::chrono::steady_clock::now();
    auto input_file = File::open_file(input);
    if(!input_file.has_value()) {
        eprintf_error("Failed to open %s", input);
        return EXIT_FAILURE;
    }
    auto input_file_data = input_file.value();

    #define TIME_ELAPSED_MS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
    
    const char *compression_format;

    if(compress_options.decompress) {
        std::vector<std::byte> decompressed_data;
        try {
            decompressed_data = Compression::decompress_map_data(input_file_data.data(), input_file_data.size());
        }
        catch(Invader::MapNeedsCompressedException &) {
            eprintf_error("Failed to decompress %s: map is already uncompressed", compress_options.output);
            return EXIT_FAILURE;
        }
        catch(std::exception &e) {
            eprintf_error("Failed to decompress %s: %s", compress_options.output, e.what());
            return EXIT_FAILURE;
        }
        if(!File::save_file(compress_options.output, decompressed_data)) {
            eprintf_error("Failed to save %s", compress_options.output);
            return EXIT_FAILURE;
        }
        auto finished = TIME_ELAPSED_MS;
        
        // Determine the compression format used
        auto &header = *reinterpret_cast<HEK::CacheFileHeader *>(decompressed_data.data());
        switch(header.engine) {
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                compression_format = COMPRESSION_FORMAT_DEFLATE;
                break;
            default:
                compression_format = COMPRESSION_FORMAT_ZSTANDARD;
        }
        
        oprintf("Decompressed %s (%s, %zu -> %zu, %zu ms)\n", input, compression_format, input_file_data.size(), decompressed_data.size(), finished);
    }
    else {
        std::vector<std::byte> compressed_data;
        try {
            compressed_data = Compression::compress_map_data(input_file_data.data(), input_file_data.size(), static_cast<int>(compress_options.compression_level));
        }
        catch(Invader::MapNeedsDecompressedException &) {
            eprintf_error("Failed to decompress %s: map is already compressed", compress_options.output);
            return EXIT_FAILURE;
        }
        catch(std::exception &e) {
            eprintf_error("Failed to compress %s: %s", compress_options.output, e.what());
            return EXIT_FAILURE;
        }
        if(!File::save_file(compress_options.output, compressed_data)) {
            eprintf_error("Failed to save %s", compress_options.output);
            return EXIT_FAILURE;
        }
        
        // Determine the compression format used
        auto &header = *reinterpret_cast<HEK::CacheFileHeader *>(input_file_data.data());
        switch(header.engine) {
            case HEK::CacheFileEngine::CACHE_FILE_XBOX:
                compression_format = COMPRESSION_FORMAT_DEFLATE;
                break;
            default:
                compression_format = COMPRESSION_FORMAT_ZSTANDARD;
        }
        
        auto finished = TIME_ELAPSED_MS;
        oprintf("Compressed %s (%s, %zu -> %zu, %.02f%%, %zu ms)\n", input, compression_format, input_file_data.size(), compressed_data.size(), compressed_data.size() * 100.0 / input_file_data.size(), finished);
    }
}
