/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <iostream>
#include <fstream>

#include <vector>
#include <cstring>

#include "../map/map.hpp"
#include "../tag/compiled_tag.hpp"
#include "../version.hpp"

#include "salamander_scenario.hpp"

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
        std::cout << "invader-scenario <scenario tag> <scenario file>" << std::endl;
        #endif
        return RETURN_FAILED_NOTHING_TO_DO;
    }

    const char *scenario_tag = argv[1];
    const char *scenario_file = argv[2];

    // Open and read input file
    FILE *f = std::fopen(scenario_file, "rb");
    if(!f) {
        std::cerr << "Failed to open " << scenario_file << "\n";
        return RETURN_FAILED_ERROR;
    }
    std::fseek(f, 0, SEEK_END);
    std::size_t size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::vector<char> scenario_file_data(size, 0);
    std::fread(scenario_file_data.data(), size, 1, f);
    std::fclose(f);

    // Open and read scenario tag
    f = std::fopen(scenario_tag, "rb");
    if(!f) {
        std::cerr << "Failed to open " << scenario_tag << "\n";
        return RETURN_FAILED_ERROR;
    }
    std::fseek(f, 0, SEEK_END);
    size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::vector<std::byte> scenario_tag_data(size, std::byte());
    std::fread(scenario_tag_data.data(), size, 1, f);

    std::cout << "Parsing file..." << std::endl;
    SalamanderScenario scenario_tag_export = parse_scenario_data(scenario_file_data);

    std::cout << "Changing tag data..." << std::endl;
    auto scenario_tag_new = scenario_to_tag(scenario_tag_data, scenario_tag_export);

    std::cout << "Writing data..." << std::endl;

    // Write scenario tag
    f = std::freopen(scenario_tag, "wb", f);
    if(!f) {
        std::cerr << "Failed to open " << scenario_tag << " for writing\n";
        return RETURN_FAILED_ERROR;
    }
    std::fseek(f, 0, SEEK_SET);
    std::fwrite(scenario_tag_new.data(), scenario_tag_new.size(), 1, f);
    std::fclose(f);
    std::cout << "Done" << std::endl;

}
