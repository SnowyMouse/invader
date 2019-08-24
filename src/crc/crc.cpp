#include <cstdio>
#include <memory>
#include "../eprintf.hpp"
#include "../map/map.hpp"
#include "hek/crc.hpp"

int main(int argc, const char **argv) {
    using namespace Invader;

    // Make sure an argument was passed. If not, give the usage
    if(argc != 2) {
        eprintf("Usage: %s <map>\n", argv[0]);
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

    // Calculate CRC32
    printf("%08X\n", calculate_map_crc(data.get(), size));
    return 0;
}
