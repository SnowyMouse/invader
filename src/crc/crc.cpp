#include <cstdio>
#include <memory>
#include <invader/printf.hpp>
#include <invader/map/map.hpp>
#include <invader/crc/hek/crc.hpp>

extern "C" {
    #include "crc_spoof.h"
    #include "crc32.h"
}

int main(int argc, const char **argv) {
    using namespace Invader;

    // Make sure an argument was passed. If not, give the usage
    if(argc < 2 || argc > 3) {
        eprintf("Usage: %s <map> [crc]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open the map
    const auto *file = argv[1];
    std::FILE *f = std::fopen(file, "rb");
    if(!f) {
        eprintf("Failed to open %s\n", file);
        return EXIT_FAILURE;
    }
    std::fseek(f, 0, SEEK_END);
    std::size_t size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    // Hold the tag data
    std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(size);
    if(std::fread(data.get(), size, 1, f) != 1) {
        eprintf("Failed to read %s\n", file);
        std::fclose(f);
        return EXIT_FAILURE;
    }
    std::fclose(f);

    if(size < sizeof(HEK::CacheFileHeader)) {
        eprintf("%s is too small to be a valid cache file.\n", file);
        return EXIT_FAILURE;
    }

    std::size_t tag_data_offset;
    std::size_t tag_data_size;
    bool has_demo_header = false;
    HEK::CacheFileEngine cache_version;
    HEK::CacheFileHeader header;
    std::memcpy(static_cast<void *>(&header), data.get(), sizeof(HEK::CacheFileHeader));
    if(header.valid()) {
        tag_data_offset = header.tag_data_offset.read();
        tag_data_size = header.tag_data_size.read();
        cache_version = header.engine.read();
    }
    else {
        // Try Demo?
        HEK::CacheFileDemoHeader demo_header;
        std::memcpy(static_cast<void *>(&demo_header), data.get(), sizeof(HEK::CacheFileDemoHeader));
        if(!demo_header.valid()) {
            eprintf("%s is not a valid cache file.\n", file);
            return EXIT_FAILURE;
        }
        has_demo_header = true;
        tag_data_offset = demo_header.tag_data_offset.read();
        tag_data_size = demo_header.tag_data_size.read();
        cache_version = demo_header.engine.read();
    }

    if(cache_version == HEK::CACHE_FILE_XBOX) {
        eprintf("%s is an Xbox cache file and not supported by invader-crc.\n", file);
        return EXIT_FAILURE;
    }

    if(size < tag_data_offset + tag_data_size) {
        eprintf("Tag data size for %s is invalid.\n", file);
        return EXIT_FAILURE;
    }

    std::uint32_t final_crc = 0;

    if(argc == 3) {
        std::size_t given_crc32_length = std::strlen(argv[2]);
        if(given_crc32_length > 8 || given_crc32_length < 1) {
            eprintf("Invalid CRC32 %s (must be 1-8 digits)\n", argv[2]);
            return EXIT_FAILURE;
        }
        for(std::size_t i = 0; i < given_crc32_length; i++) {
            char c = std::tolower(argv[2][i]);
            if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                eprintf("Invalid CRC32 %s (must be hexadecimal)\n", argv[2]);
                return EXIT_FAILURE;
            }
        }

        std::uint32_t given_crc = std::strtoul(argv[2], nullptr, 16);
        std::uint32_t checksum_delta;
        final_crc = calculate_map_crc(data.get(), size, &given_crc, &checksum_delta);

        std::size_t crc32_offset = offsetof(HEK::CacheFileHeader, crc32);
        if(has_demo_header) {
            crc32_offset = offsetof(HEK::CacheFileDemoHeader, crc32);
        }

        f = std::fopen(file, "rb+");
        if(!f) {
            eprintf("Failed to open %s\n", file);
            return EXIT_FAILURE;
        }

        // Write header crc32
        std::fseek(f, crc32_offset, SEEK_SET);
        std::fwrite(&final_crc, sizeof(final_crc), 1, f);
        // Write delta to tag header
        std::fseek(f, tag_data_offset + offsetof(HEK::CacheFileTagDataHeader, tag_file_checksums), SEEK_SET);
        std::fwrite(&checksum_delta, sizeof(checksum_delta), 1, f);
        std::fclose(f);
    }
    else {
        final_crc = calculate_map_crc(data.get(), size);
    }

    // Calculate CRC32
    printf("%08X\n", final_crc);
    return EXIT_SUCCESS;
}
