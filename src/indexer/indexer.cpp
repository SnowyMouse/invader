/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef NO_OUTPUT
#include <iostream>
#include <chrono>
using clock_type = std::chrono::steady_clock;
#endif

#include <vector>
#include <cstring>

#include "../map/map.hpp"
#include "../tag/compiled_tag.hpp"
#include "../version.hpp"

enum ReturnValue : int {
    RETURN_OK = 0,
    RETURN_FAILED_NOTHING_TO_DO = 1,
    RETURN_FAILED_ERROR = 2
};

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;

    if(argc < 3) {
        #ifndef NO_OUTPUT
        std::cout << "invader-indexer <input map> <output map>"                                         << std::endl;
        #endif
        return RETURN_FAILED_NOTHING_TO_DO;
    }

    const char *output = argv[2];
    const char *input = argv[1];

    // Open input map
    FILE *f = std::fopen(input, "rb");
    if(!f) {
        std::cerr << "Failed to open " << input << "\n";
        return RETURN_FAILED_ERROR;
    }
    std::fseek(f, 0, SEEK_END);
    std::size_t size = std::ftell(f);
    std::unique_ptr<std::byte> map_data(new std::byte[size]);
    std::fseek(f, 0, SEEK_SET);
    if(std::fread(map_data.get(), size, 1, f) != 1) {
        std::cerr << "Failed to read " << argv[1] << "\n";
        return RETURN_FAILED_ERROR;
    }
    std::fclose(f);

    f = nullptr;

    try {
        Map map(map_data.get(), size);

        // Open output
        f = std::fopen(output, "wb");
        if(!f) {
            std::cerr << "Failed to open " << output << "\n";
            return RETURN_FAILED_ERROR;
        }

        auto tag_count = map.tag_count();
        std::fprintf(f, "%zu\n", tag_count);
        for(std::size_t i = 0; i < tag_count; i++) {
            auto &tag = map.get_tag(i);
            std::fprintf(f, "%u\t%s\n", tag.tag_class_int(), tag.path().data());
        }

        std::fclose(f);
    }
    catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        std::fclose(f);
        return RETURN_FAILED_ERROR;
    }
}
