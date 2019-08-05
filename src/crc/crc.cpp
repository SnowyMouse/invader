#include <cstdio>
#include <memory>
#include <vector>
#include "../eprintf.hpp"
#include "../map/map.hpp"
#include "../tag/hek/class/scenario.hpp"

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
    std::vector<std::byte> data_crc(0);
    data_crc.reserve(size);
    if(std::fread(data.get(), size, 1, f) != 1) {
        eprintf("Failed to read %s\n", file);
        std::fclose(f);
        return 1;
    }
    std::fclose(f);

    // Parse the map
    Map map = Map::map_with_pointer(data.get(), size);
    auto &scenario_tag = map.get_tag(map.get_scenario_tag_id());
    auto &scenario = scenario_tag.get_base_struct<HEK::Scenario>();
    std::size_t bsp_count = scenario.structure_bsps.count.read();
    auto *bsps = scenario_tag.resolve_reflexive(scenario.structure_bsps);

    // Iterate through all BSPs
    for(std::size_t b = 0; b < bsp_count; b++) {
        std::size_t start = bsps[b].bsp_start.read();
        std::size_t end = start + bsps[b].bsp_size.read();

        if(start >= size || end > size) {
            eprintf("Failed to CRC bsp #%zu\n", b);
            return 1;
        }

        // Add it
        data_crc.insert(data_crc.end(), data.get() + start, data.get() + end);
    }

    // Now copy model data
    const auto &tag_data_header = static_cast<const HEK::CacheFileTagDataHeaderPC &>(map.get_tag_data_header());
    std::size_t model_start = tag_data_header.model_data_file_offset.read();
    std::size_t model_end = model_start + tag_data_header.model_data_size.read();
    if(model_start >= size || model_end > size) {
        eprintf("Failed to CRC model data\n");
        return 1;
    }
    data_crc.insert(data_crc.end(), data.get() + model_start, data.get() + model_end);

    // Lastly, do tag data
    const auto &cache_file_header = map.get_cache_file_header();
    std::size_t tag_data_start = cache_file_header.tag_data_offset.read();
    std::size_t tag_data_end = tag_data_start + cache_file_header.tag_data_size.read();
    if(tag_data_start >= size || tag_data_end > size) {
        eprintf("Failed to CRC tag data\n");
        return 1;
    }

    // Find out where we're going to be doing CRC32 stuff
    const std::byte *random_number_ptr = reinterpret_cast<const std::byte *>(&tag_data_header.random_number);
    std::size_t random_number_offset_in_memory = random_number_ptr - reinterpret_cast<const std::byte *>(&tag_data_header) + data_crc.size();
    std::size_t random_number_offset_in_file = random_number_ptr - data.get();
    data_crc.insert(data_crc.end(), data.get() + tag_data_start, data.get() + tag_data_end);

    // Overwrite with new CRC32
    if(argc == 3) {
        // Make sure the CRC32 given is actually valid
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

        FakeFileHandle handle = { reinterpret_cast<std::uint8_t *>(data_crc.data()), data_crc.size(), 0 };
        std::uint32_t desired_crc = static_cast<std::uint32_t>(std::strtoul(argv[2], nullptr, 16));
        std::uint32_t newcrc = ~crc_spoof_reverse_bits(desired_crc);
        crc_spoof_modify_file_crc32(&handle, random_number_offset_in_memory, newcrc, false);

        f = std::fopen(argv[1], "r+");
        if(!f) {
            eprintf("Failed to open for reading/writing\n");
            return 1;
        }
        std::fseek(f, random_number_offset_in_file, SEEK_SET);
        if(!fwrite(reinterpret_cast<std::byte *>(data_crc.data() + random_number_offset_in_memory), sizeof(std::uint32_t), 1, f)) {
            eprintf("Failed to update value\n");
            return 1;
        }
        std::fclose(f);
    }

    // Calculate CRC32
    printf("%08X\n", ~crc32(0, data_crc.data(), data_crc.size()));
    return 0;
}
