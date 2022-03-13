// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include "../command_line_option.hpp"
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>
#include <invader/map/map.hpp>

int main(int argc, char * const *argv) {
    set_up_color_term();
    
    using namespace Invader;
    
    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO)
    };

    static constexpr char DESCRIPTION[] = "Scans for unknown hidden data in tags";
    static constexpr char USAGE[] = "[options] <map>";

    struct ScanOptions {
    } scan_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<ScanOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, scan_options, [](char opt, const std::vector<const char *> &, ScanOptions &) {
        switch(opt) {
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });
    
    auto map_data = File::open_file(remaining_arguments[0]).value();
    auto map = Map::map_with_copy(map_data.data(), map_data.size());
    auto tag_count = map.get_tag_count();
    
    for(std::size_t t = 0; t < tag_count; t++) {
        auto &tag = map.get_tag(t);
        if(!tag.data_is_available()) {
            continue;
        }
        
        #define DO_TAG_CLASS(c, v) case HEK::v: {\
            Parser::c::scan_padding(tag);\
            break;\
        }
        
        auto tci = tag.get_tag_fourcc();
        if(tci == HEK::TagFourCC::TAG_FOURCC_SCENARIO_STRUCTURE_BSP && map.get_cache_version() != HEK::CacheFileEngine::CACHE_FILE_NATIVE) {
            Parser::ScenarioStructureBSP::scan_padding(tag, tag.get_base_struct<HEK::ScenarioStructureBSPCompiledHeader>().pointer);
            continue;
        }
        
        switch(tci) {
            DO_BASED_ON_TAG_CLASS
            default: break;
        }
    }
}
