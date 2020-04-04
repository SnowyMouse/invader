// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>

#include <invader/map/map.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>

template <typename T> static void close_input(T &options) {
    if(options.top_input && !options.top_input->map.has_value() && options.top_input->tags.size() == 0) {
        eprintf_error("Inputs must have either a tags directory or a map.");
        std::exit(EXIT_FAILURE);
    }
    options.top_input = nullptr;
}

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;

    struct CompareOptions {
        struct Input {
            std::optional<const char *> map;
            std::optional<std::string> maps;
            std::vector<std::string> tags;
        };
        
        std::vector<HEK::TagClassInt> class_to_check;
        Input *top_input = nullptr;
        std::vector<Input> inputs;
        bool exhaustive = false;
        bool open = false;
    } compare_options;

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("input", 'I', 0, "Add an input directory");
    options.emplace_back("tags", 't', 1, "Add a tags directory to the input. Specify multiple tag directories in order of precedence for the input.");
    options.emplace_back("maps", 'm', 1, "Add a maps directory to the input to specify where to find resource files for a map. This cannot be used with --tags.");
    options.emplace_back("map", 'M', 1, "Add a map to the input. Only one map can be specified per input. If a maps directory isn't specified, then the map's directory will be used. This cannot be used with --tags.");
    options.emplace_back("class", 'c', 1, "Add a tag class to check. If no tag classes are specified, all tag classes will be checked.");

    static constexpr char DESCRIPTION[] = "Create a file listing the tags of a map.";
    static constexpr char USAGE[] = "[options] <-I <options>> <-I <options>> [<-I <options>> ...]";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<CompareOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, compare_options, [](char opt, const auto &args, CompareOptions &compare_options) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
                
            case 'I':
                close_input(compare_options);
                compare_options.top_input = &compare_options.inputs.emplace_back();
                break;
                
            case 'm':
                if(!compare_options.top_input) {
                    eprintf_error("An input is required before setting a maps directory.");
                    std::exit(EXIT_FAILURE);
                }
                if(compare_options.top_input->maps.has_value()) {
                    eprintf_error("A maps directory was already specified for this input.");
                    std::exit(EXIT_FAILURE);
                }
                compare_options.top_input->maps = args[0];
                break;
                
            case 'M':
                if(!compare_options.top_input) {
                    eprintf_error("An input is required before setting a maps directory.");
                    std::exit(EXIT_FAILURE);
                }
                if(compare_options.top_input->tags.size() > 0) {
                    eprintf_error("A tags directory was already specified for this input.");
                    std::exit(EXIT_FAILURE);
                }
                if(compare_options.top_input->map.has_value()) {
                    eprintf_error("A map was already specified for this input.");
                    std::exit(EXIT_FAILURE);
                }
                compare_options.top_input->map = args[0];
                break;
                
            case 't':
                if(!compare_options.top_input) {
                    eprintf_error("An input is required before setting a tags directory.");
                    std::exit(EXIT_FAILURE);
                }
                if(compare_options.top_input->map.has_value()) {
                    eprintf_error("A map was already specified for this input.");
                    std::exit(EXIT_FAILURE);
                }
                if(compare_options.top_input->maps.has_value()) {
                    eprintf_error("A maps directory was already specified for this input.");
                    std::exit(EXIT_FAILURE);
                }
                compare_options.top_input->tags.emplace_back(args[0]);
                break;
                
            case 'c':
                auto class_to_check = extension_to_tag_class(args[0]);
                for(auto c : compare_options.class_to_check) {
                    if(c == class_to_check) {
                        eprintf_error("Class %s was already specified", args[0]);
                        std::exit(EXIT_FAILURE);
                    }
                }
                if(class_to_check == TagClassInt::TAG_CLASS_NULL || class_to_check == TagClassInt::TAG_CLASS_NONE) {
                    eprintf_error("Class %s is not a valid class", args[0]);
                }
                compare_options.class_to_check.push_back(class_to_check);
                break;
                
        }
    });
    
    // Make sure everything's valid
    if(compare_options.inputs.size() < 2) {
        eprintf_error("At least two inputs are required");
        return EXIT_FAILURE;
    }
    
    // Can we close it?
    close_input(compare_options);
    
    // Automatically make up maps directories for any map when necessary
    for(auto &i : compare_options.inputs) {
        if(i.map.has_value() && !i.maps.has_value()) {
            i.maps = std::filesystem::absolute(std::filesystem::path(*i.map)).parent_path().string();
            oprintf("%s\n", i.maps.value().c_str());
        }
    }
}
