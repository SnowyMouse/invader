#include <cstdio>
#include <memory>
#include "../eprintf.hpp"
#include "../map/map.hpp"
#include "hek/crc.hpp"

extern "C" {
    #include "crc_spoof.h"
    #include "crc32.h"
}

int main(int argc, const char **argv) {
    using namespace Invader;

    // Make sure an argument was passed. If not, give the usage
    if(argc < 2 || argc > 3) {
        eprintf("%s <map> [crc]\n", argv[0]);
        return 1;
    }

    // Open the map
    const auto *file = argv[1];
    std::FILE *f = std::fopen(file, "rb");
    if(!f) {
        eprintf("Failed to open %s\n", file);
        return 1;
    }
    std::fseek(f, 0, SEEK_END);
    std::size_t size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    // Hold the tag data
    std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(size);
    if(std::fread(data.get(), size, 1, f) != 1) {
        eprintf("Failed to read %s\n", file);
        std::fclose(f);
        return 1;
    }
    std::fclose(f);

    std::uint32_t final_crc = 0;

    if(argc == 3) {
        std::size_t given_crc32_length = std::strlen(argv[2]);
        if(given_crc32_length > 8 || given_crc32_length < 1) {
            eprintf("Invalid CRC32 %s (must be 1-8 digits)\n", argv[2]);
            return 1;
        }
        for(std::size_t i = 0; i < given_crc32_length; i++) {
            char c = std::tolower(argv[2][i]);
            if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                eprintf("Invalid CRC32 %s (must be hexadecimal)\n", argv[2]);
                return 1;
            }
        }

        std::uint32_t given_crc = std::strtoul(argv[2], nullptr, 16);
        std::uint32_t new_random;
        final_crc = calculate_map_crc(data.get(), size, &given_crc, &new_random);

        f = std::fopen(file, "rb+");
        if(!f) {
            eprintf("Failed to open %s\n", file);
            return 1;
        }

        HEK::CacheFileTagDataHeader header;
        std::fseek(f, *reinterpret_cast<std::uint32_t *>(data.get() + 0x10) + static_cast<long>(reinterpret_cast<std::byte *>(&header.random_number) - reinterpret_cast<std::byte *>(&header)), SEEK_CUR);
        std::fwrite(&new_random, sizeof(new_random), 1, f);
        std::fclose(f);
    }
    else {
        final_crc = calculate_map_crc(data.get(), size);
    }

    // Calculate CRC32
    printf("%08X\n", final_crc);
    return 0;
}
