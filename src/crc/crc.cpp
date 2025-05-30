#include <cstdio>
#include <memory>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/map/map.hpp>
#include <invader/crc/hek/crc.hpp>
#include "../command_line_option.hpp"

extern "C" {
    #include "crc_spoof.h"
    #include "crc32.h"
}

using namespace Invader;

static std::uint32_t read_str32(const char *err, const char *s) {
    // Make sure it starts with '0x'
    if(std::strncmp(s, "0x", 2) != 0) {
        eprintf_error("%s %s (must be hexadecimal)", err, s);
        std::exit(EXIT_FAILURE);
    }

    // Check the string length
    std::size_t given_crc32_length = std::strlen(s);
    if(given_crc32_length > 10 || given_crc32_length < 3) {
        eprintf_error("%s %s (must be 1-8 digits)", err, s);
        std::exit(EXIT_FAILURE);
    }

    // Now, make sure it's all valid
    for(std::size_t i = 2; i < given_crc32_length; i++) {
        char c = std::tolower(s[i]);
        if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
            eprintf_error("%s %s (must be hexadecimal)", err, s);
            std::exit(EXIT_FAILURE);
        }
    }

    return static_cast<std::uint32_t>(std::strtoul(s + 2, nullptr, 16));
}

static void update_cache_file(const auto file, bool demo_header, std::uint32_t crc32, std::optional<std::size_t> tag_data_offset = std::nullopt, std::optional<std::uint32_t> delta = std::nullopt) {
    std::size_t crc32_offset = offsetof(HEK::CacheFileHeader, crc32);
    if(demo_header) {
        crc32_offset = offsetof(HEK::CacheFileDemoHeader, crc32);
    }

    std::FILE *f = std::fopen(file, "rb+");
    if(!f) {
        eprintf_error("Failed to open %s", file);
        std::exit(EXIT_FAILURE);
    }

    // Write header crc32
    std::fseek(f, crc32_offset, SEEK_SET);
    std::fwrite(&crc32, sizeof(crc32), 1, f);

    // Write delta to tag header if we need to
    if(delta) {
        std::fseek(f, tag_data_offset.value() + offsetof(HEK::CacheFileTagDataHeader, tag_file_checksums), SEEK_SET);
        std::fwrite(&delta.value(), sizeof(delta.value()), 1, f);
    }
    std::fclose(f);
}

int main(int argc, const char **argv) {
    set_up_color_term();

    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption("forge", 'c', 1, "Forge the CRC32 value of the map.", "<crc>"),
        CommandLineOption("fix-header", 'f', 0, "Set the CRC32 field in the cache file header to the current calculated value if mismatched. This is implied when using --forge")
    };

    static constexpr char DESCRIPTION[] = "Inspect or forge the CRC32 of a cache file.";
    static constexpr char USAGE[] = "[options] <map>";

    struct CrcOptions {
        bool fix_header = false;
        std::optional<std::uint32_t> forge_crc;
    } crc_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<CrcOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, crc_options, [](char opt, const auto &arguments, auto &crc_options) {
        switch(opt) {
            case 'c':
                crc_options.forge_crc = read_str32("Invalid CRC32", arguments[0]);
                break;
            case 'f':
                crc_options.fix_header = true;
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                break;
        }
    });

    // Open the map
    const auto *file = remaining_arguments[0];
    std::FILE *f = std::fopen(file, "rb");
    if(!f) {
        eprintf_error("Failed to open %s", file);
        return EXIT_FAILURE;
    }
    std::fseek(f, 0, SEEK_END);
    std::size_t size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    // Hold the tag data
    std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(size);
    if(std::fread(data.get(), size, 1, f) != 1) {
        eprintf_error("Failed to read %s", file);
        std::fclose(f);
        return EXIT_FAILURE;
    }
    std::fclose(f);

    if(size < sizeof(HEK::CacheFileHeader)) {
        eprintf_error("%s is too small to be a valid cache file.", file);
        return EXIT_FAILURE;
    }

    std::size_t tag_data_offset;
    std::size_t tag_data_size;
    bool has_demo_header = false;
    HEK::CacheFileEngine cache_version;
    HEK::CacheFileHeader header;
    std::uint32_t header_crc;
    std::memcpy(static_cast<void *>(&header), data.get(), sizeof(HEK::CacheFileHeader));
    if(header.valid()) {
        tag_data_offset = header.tag_data_offset.read();
        tag_data_size = header.tag_data_size.read();
        cache_version = header.engine.read();
        header_crc = header.crc32.read();
    }
    else {
        // Try Demo?
        HEK::CacheFileDemoHeader demo_header;
        std::memcpy(static_cast<void *>(&demo_header), data.get(), sizeof(HEK::CacheFileDemoHeader));
        if(!demo_header.valid()) {
            eprintf_error("%s is not a valid cache file.", file);
            return EXIT_FAILURE;
        }
        has_demo_header = true;
        tag_data_offset = demo_header.tag_data_offset.read();
        tag_data_size = demo_header.tag_data_size.read();
        cache_version = demo_header.engine.read();
        header_crc = demo_header.crc32.read();
    }

    if(cache_version == HEK::CACHE_FILE_XBOX) {
        eprintf_error("%s is an Xbox cache file and not supported by invader-crc.", file);
        return EXIT_FAILURE;
    }

    if(size < tag_data_offset + tag_data_size) {
        eprintf_error("Tag data size for %s is invalid.", file);
        return EXIT_FAILURE;
    }

    std::uint32_t final_crc = 0;

    if(crc_options.forge_crc.has_value()) {
        // Forge the CRC32 to a new value
        std::uint32_t checksum_delta;
        final_crc = calculate_map_crc(data.get(), size, &crc_options.forge_crc.value(), &checksum_delta);
        update_cache_file(file, has_demo_header, final_crc, tag_data_offset, checksum_delta);
        oprintf_success("Successfully forged CRC32 to 0x%08X", final_crc);
    }
    else {
        // Calculate CRC32
        final_crc = calculate_map_crc(data.get(), size);
        if(crc_options.fix_header) {
            if(final_crc != header_crc) {
                update_cache_file(file, has_demo_header, final_crc);
                oprintf_success("Mismatched header CRC32 (0x%08X) was set to 0x%08X", header_crc, final_crc);
            }
            else {
                oprintf("CRC32 0x%08X matches header (nothing to fix)\n", final_crc);
            }
        }
        else {
            oprintf("0x%08X\n", final_crc);
        }
    }

    return EXIT_SUCCESS;
}
