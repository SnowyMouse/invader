// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <memory>
#include "invader/printf.hpp"
#include "invader/map/map.hpp"
#include "invader/crc/hek/crc.hpp"

int main(int argc, const char **argv) {
    using namespace Invader;

    // Make sure an argument was passed. If not, give the usage
    if(argc != 2) {
        eprintf("Usage: %s <map>\n", argv[0]);
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

    // Calculate CRC32
    if(size < 0x800) {
        eprintf("Error: Invalid map\n");
        return EXIT_FAILURE;
    }

    // Check if it's obviously a resource map
    else if(*reinterpret_cast<std::uint32_t *>(data.get()) < 256) {
        eprintf("Error: Resource maps cannot be used with this tool.\n");
        return EXIT_FAILURE;
    }

    else {
        try {
            bool dirty;
            auto map_crc = calculate_map_crc(data.get(), size, nullptr, nullptr, &dirty, true);
            std::printf("%08X\n", map_crc);
            if(dirty) {
                eprintf("Warning: Cache file is dirty. The CRC in the header is wrong.\n");
            }
            return EXIT_SUCCESS;
        }
        catch(std::exception &e) {
            eprintf("Failed to find the CRC for %s\n", argv[1]);
            eprintf("%s\n", e.what());
            return EXIT_FAILURE;
        }
    }
}
