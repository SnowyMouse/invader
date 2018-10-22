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

    //const char *scenario_tag = argv[1];
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

    SalamanderScenario scenario_tag_export = scenario_from_file(scenario_file_data);
}
