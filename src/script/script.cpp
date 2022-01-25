// SPDX-License-Identifier: GPL-3.0-only

#include <map>
#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/command_line_option.hpp>
#include <invader/version.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/compile/scenario.hpp>
#include "../build/build.hpp"

#include <riat/riat.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;

    struct ScriptOption {
        const char *path;
        std::filesystem::path data = "data";
        std::filesystem::path tags = "tags";
        bool regenerate = false;
        bool clear = false;
        bool filesystem_path = false;
        const GameEngineInfo *engine = &GameEngineInfo::get_game_engine_info(GameEngine::GAME_ENGINE_NATIVE);
    } script_options;
    script_options.path = *argv;
    
    std::string game_engine_arguments = std::string("Specify the game engine. Valid engines are: ") + Build::get_comma_separated_game_engine_shorthands();

    // Add our options
    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("clear", 'c', 0, "Clear all script data from the scenario tag");
    options.emplace_back("game-engine", 'g', 1, game_engine_arguments.c_str(), "<engine>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path directory.");
    options.emplace_back("regenerate", 'R', 0, "Use the scenario tag's script source data as data.");
    options.emplace_back("tags", 't', 1);

    static constexpr char DESCRIPTION[] = "Compile scripts.";
    static constexpr char USAGE[] = "[options] <scenario>";

    // Parse arguments
    auto remaining_arguments = CommandLineOption::parse_arguments<ScriptOption &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, script_options, [](char opt, const auto &arguments, ScriptOption &script_options) {
        switch(opt) {
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                
            case 'g': {
                if(const auto *engine_maybe = GameEngineInfo::get_game_engine_info(arguments[0])) {
                    script_options.engine = engine_maybe;
                }
                else {
                    eprintf_error("Unknown engine %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                
                break;
            }

            case 'd':
                script_options.data = arguments[0];
                break;

            case 'c':
                script_options.clear = true;
                break;
                
            case 'R':
                script_options.regenerate = true;
                break;

            case 'P':
                script_options.filesystem_path = true;
                break;

            case 't':
                script_options.tags = arguments[0];
                break;
        }
    });

    // Get the scenario tag
    std::string scenario;
    if(remaining_arguments.size() == 0) {
        eprintf("Expected a scenario tag. Use -h for more information.\n");
        return EXIT_FAILURE;
    }
    else if(remaining_arguments.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_arguments[1]);
        return EXIT_FAILURE;
    }
    else {
        if(script_options.filesystem_path) {
            auto tag_maybe = File::file_path_to_tag_path(remaining_arguments[0], script_options.tags);
            auto split = File::split_tag_class_extension(tag_maybe.value_or(""));
            
            
            auto tag_path = File::file_path_to_tag_path(remaining_arguments[0], script_options.tags);
            
            if(split.has_value() && std::filesystem::exists(remaining_arguments[0])) {
                scenario = split->path;
            }
            else {
                eprintf_error("Failed to find a valid tag %s in %s.", remaining_arguments[0], script_options.tags.string().c_str());
                return EXIT_FAILURE;
            }
        }
        else {
            scenario = File::halo_path_to_preferred_path(remaining_arguments[0]);
        }
    }

    auto tag_path = script_options.tags / (scenario + ".scenario");
    auto script_directory_path = (script_options.data / scenario).parent_path() / "scripts";
    auto global_script_path = script_options.data / "global_scripts.hsc";
    
    // Open the scenario tag
    auto scenario_file_data = File::open_file(tag_path);
    if(!scenario_file_data.has_value()) {
        eprintf("Failed to open %s\n", tag_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Parse it
    Parser::Scenario s;
    try {
        s = Parser::Scenario::parse_hek_tag_file((*scenario_file_data).data(), (*scenario_file_data).size());
    }
    catch(std::exception &e) {
        eprintf_error("Failed to parse %s: %s", tag_path.string().c_str(), e.what());
        return EXIT_FAILURE;
    }
    
    std::optional<std::vector<std::pair<std::string, std::vector<std::byte>>>> source_files;
    auto load_script = [&source_files](const std::filesystem::path &path) {
        auto filename = path.filename();
        auto filename_without_extension = std::filesystem::path(filename).replace_extension().string();
        
        // Open it!
        auto f = File::open_file(path).value();
        
        // Move it into here
        source_files->emplace_back(filename_without_extension, std::move(f));
    };
    
    try {
        std::vector<std::string> warnings;
        
        // If we're clearing, supply an empty array
        if(script_options.clear) {
            source_files.emplace();
        }
        
        // If we aren't regenerating, load the scripts in here
        else if(!script_options.regenerate) {
            // Instantiate our array
            source_files.emplace();
            
            // First load the global scripts
            if(std::filesystem::exists(global_script_path)) {
                load_script(global_script_path);
            }
            
            // Next, load scripts in script directory
            std::list<std::filesystem::path> script_paths;
            
            if(std::filesystem::exists(script_directory_path)) {
                for(auto &p : std::filesystem::directory_iterator(script_directory_path)) {
                    auto path = p.path();
                    if(!p.is_regular_file() || path.extension() != ".hsc") {
                        continue;
                    }
                    script_paths.emplace_back(path);
                }
            }
            script_paths.sort();
            
            // Load in that order
            for(auto &path : script_paths) {
                load_script(path);
            }
        }
        
        // Compile
        Parser::compile_scripts(s, *script_options.engine, RIAT_OptimizationLevel::RIAT_OPTIMIZATION_PREVENT_GENERATIONAL_LOSS, warnings, source_files);
        
        for(auto &w : warnings) {
            eprintf_warn("%s", w.c_str());
        }
    }
    catch(std::exception &e) {
        eprintf_error("%s", e.what());
        return EXIT_FAILURE;
    }
    
    std::size_t script_count = s.scripts.size();
    std::size_t global_count = s.globals.size();
    
    // Warn if the user may have messed up something
    if(!script_options.clear && source_files.has_value() && source_files->size() == 0) {
        oprintf_success_warn("WARNING: No source files were compiled");
    }
    
    if(!script_options.clear) {
        oprintf("Compiled %zu script%s and %zu global%s\n", script_count, script_count == 1 ? "" : "s", global_count, global_count == 1 ? "" : "s");
    }
        
    // Write
    auto output = s.generate_hek_tag_data(TagFourCC::TAG_FOURCC_SCENARIO);
    if(File::save_file(tag_path, output)) {
        oprintf_success("Successfully %s scripts", script_options.clear ? "cleared" : "compiled");
        return EXIT_SUCCESS;
    }
    else {
        eprintf_error("Failed to write to %s", tag_path.string().c_str());
        return EXIT_FAILURE;
    }
}
