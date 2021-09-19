// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/command_line_option.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/parser/definition/all.hpp>
#include <invader/file/file.hpp>
#include <invader/map/map.hpp>

using namespace Invader::Parser;

int main(int argc, char * const *argv) {
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");

    static constexpr char DESCRIPTION[] = "Scans for unknown hidden data in tags";
    static constexpr char USAGE[] = "[options] <map>";

    struct ScanOptions {
    } scan_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<ScanOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, scan_options, [](char opt, const std::vector<const char *> &, ScanOptions &) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });
    
    auto map_data = Invader::File::open_file(remaining_arguments[0]).value();
    auto map = Invader::Map::map_with_copy(map_data.data(), map_data.size());
    auto tag_count = map.get_tag_count();
    
    for(std::size_t t = 0; t < tag_count; t++) {
        auto &tag = map.get_tag(t);
        if(!tag.data_is_available()) {
            continue;
        }
        
        #define DO_TAG_CLASS(c, v) case v: {\
            c::scan_padding(tag);\
            break;\
        }
        
        auto tci = tag.get_tag_fourcc();
        if(tci == TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP && map.get_cache_version() != CacheFileEngine::CACHE_FILE_NATIVE) {
            ScenarioStructureBSP::scan_padding(tag, tag.get_base_struct<ScenarioStructureBSPCompiledHeader::C>().pointer);
            continue;
        }
        
        switch(tci) {
            DO_BASED_ON_TAG_CLASS
            default: break;
        }
    }
}
