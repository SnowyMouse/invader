#include <chrono>

#include <zstd.h>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include <invader/compress/compression.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;

    struct CompressOptions {
        const char *output = nullptr;
        long compression_level = 3;
        bool decompress = false;
    } compress_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("output", 'o', 1, "Emit the resulting map at the given path. By default, this is the map path (overwrite).", "<file>");
    options.emplace_back("level", 'l', 1, "Set the compression level. Must be between 1 and 22. Values > 19 use more memory. Default: 3", "<level>");
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
                if(compress_options.compression_level < 1 || compress_options.compression_level > 22) {
                    eprintf("Compression level must be between 1 and 22\n");
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
        eprintf("Failed to open %s\n", input);
        return EXIT_FAILURE;
    }
    auto input_file_data = input_file.value();

    #define TIME_ELAPSED_MS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()

    if(compress_options.decompress) {
        auto decompressed_data = Compression::decompress_map_data(input_file_data.data(), input_file_data.size());
        if(!File::save_file(compress_options.output, decompressed_data)) {
            eprintf("Failed to save %s\n", compress_options.output);
            return EXIT_FAILURE;
        }
        auto finished = TIME_ELAPSED_MS;
        oprintf("Decompressed %s (%zu -> %zu, %zu ms)\n", input, input_file_data.size(), decompressed_data.size(), finished);
    }
    else {
        auto compressed_data = Compression::compress_map_data(input_file_data.data(), input_file_data.size(), static_cast<int>(compress_options.compression_level));
        if(!File::save_file(compress_options.output, compressed_data)) {
            eprintf("Failed to save %s\n", compress_options.output);
            return EXIT_FAILURE;
        }
        auto finished = TIME_ELAPSED_MS;
        oprintf("Compressed %s (%zu -> %zu, %.02f%%, %zu ms)\n", input, input_file_data.size(), compressed_data.size(), compressed_data.size() * 100.0 / input_file_data.size(), finished);
    }
}
