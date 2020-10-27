// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>

#include <invader/map/map.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/extract/extraction.hpp>
#include <invader/command_line_option.hpp>

using namespace Invader;

enum Show : std::uint8_t {
    SHOW_MISMATCHED = 1 << 0,
    SHOW_MATCHED = 1 << 1,
    SHOW_ALL = 0xFF
};

struct Input {
    std::optional<const char *> map;
    std::optional<std::filesystem::path> maps;
    std::vector<std::string> tags;
    bool ignore_resource_maps = false;
    
    std::vector<File::TagFilePath> tag_paths;
    std::vector<File::TagFile> virtual_directory;
    std::unique_ptr<Map> map_data;
};

template <typename T> static void close_input(T &options) {
    if(options.top_input && !options.top_input->map.has_value() && options.top_input->tags.size() == 0) {
        eprintf_error("Inputs must have either a tags directory or a map.");
        std::exit(EXIT_FAILURE);
    }
    options.top_input = nullptr;
}

enum ByPath {
    BY_PATH_SAME = 0,
    BY_PATH_ANY = 1,
    BY_PATH_DIFFERENT = 2
};

static void regular_comparison(const std::vector<Input> &inputs, bool precision, Show show, bool match_all, bool functional, ByPath by_path);

int main(int argc, const char **argv) {
    using namespace Invader::HEK;

    struct CompareOptions {
        std::vector<HEK::TagClassInt> class_to_check;
        Input *top_input = nullptr;
        std::vector<Input> inputs;
        bool exhaustive = false;
        bool precision = false;
        bool match_all = false;
        bool functional = false;
        ByPath by_path = ByPath::BY_PATH_SAME;
        Show show = Show::SHOW_ALL;
    } compare_options;

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("input", 'I', 0, "Add an input directory");
    options.emplace_back("tags", 't', 1, "Add a tags directory to the input. Specify multiple tag directories in order of precedence for the input.");
    options.emplace_back("maps", 'm', 1, "Add a maps directory to the input to specify where to find resource files for a map.");
    options.emplace_back("map", 'M', 1, "Add a map to the input. Only one map can be specified per input. If a maps directory isn't specified, then the map's directory will be used.");
    options.emplace_back("class", 'c', 1, "Add a tag class to check. If no tag classes are specified, all tag classes will be checked.");
    options.emplace_back("precision", 'p', 0, "Allow for slight differences in floats to account for precision loss.");
    options.emplace_back("functional", 'f', 0, "Precompile the tags before comparison to check for only functional differences.");
    options.emplace_back("by-path", 'B', 1, "Set what tags get compared against other tags. By default, only tags with the same relative path are checked. Using \"any\" ignores paths completely (useful for finding duplicates when both inputs are different) while \"different\" only checks tags with different paths (useful for finding duplicates when both inputs are the same). Can be: any, different, or same (default)", "<path-type>");
    options.emplace_back("show", 's', 1, "Can be: all, matched, or mismatched. Default: all");
    options.emplace_back("ignore-resources", 'G', 0, "Ignore resource maps for the current map input.");
    options.emplace_back("all", 'a', 0, "Only match if tags are in all inputs");

    static constexpr char DESCRIPTION[] = "Compare tags against other tags.";
    static constexpr char USAGE[] = "[options] <-I <options>> <-I <options>> [<-I <options>> ...]";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<CompareOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, compare_options, [](char opt, const auto &args, CompareOptions &compare_options) {
        static const char *CAN_ONLY_BE_USED_WITH_TAG_INPUT = "This option can only be used with a tag input.";
        static const char *CAN_ONLY_BE_USED_WITH_MAP_INPUT = "This option can only be used with a map input.";
        
        bool top_option_is_tag_input = false;
        bool top_option_is_map_input = false;
        
        if(compare_options.top_input) {
            top_option_is_tag_input = !compare_options.top_input->maps.has_value() && !compare_options.top_input->map.has_value();
            top_option_is_map_input = !compare_options.top_input->tags.size();
        }
        
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
                
            case 'G':
                if(!top_option_is_map_input) {
                    eprintf_error("%s", CAN_ONLY_BE_USED_WITH_MAP_INPUT);
                    std::exit(EXIT_FAILURE);
                }
                compare_options.top_input->ignore_resource_maps = true;
                break;
                
            case 'B':
                if(std::strcmp(args[0], "any") == 0) {
                    compare_options.by_path = ByPath::BY_PATH_ANY;
                }
                else if(std::strcmp(args[0], "different") == 0) {
                    compare_options.by_path = ByPath::BY_PATH_DIFFERENT;
                }
                else if(std::strcmp(args[0], "same") == 0) {
                    compare_options.by_path = ByPath::BY_PATH_SAME;
                }
                else {
                    eprintf_error("Unknown by-path argument %s", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
                
            case 'I':
                close_input(compare_options);
                compare_options.top_input = &compare_options.inputs.emplace_back();
                break;
                
            case 'm':
                if(!compare_options.top_input) {
                    eprintf_error("An input is required before setting a maps directory.");
                    std::exit(EXIT_FAILURE);
                }
                if(!top_option_is_tag_input) {
                    eprintf_error("%s", CAN_ONLY_BE_USED_WITH_TAG_INPUT);
                    std::exit(EXIT_FAILURE);
                }
                if(compare_options.top_input->maps.has_value()) {
                    eprintf_error("A maps directory was already specified for this input.");
                    std::exit(EXIT_FAILURE);
                }
                compare_options.top_input->maps = args[0];
                break;
                
            case 'f':
                compare_options.functional = true;
                break;
                
            case 'M':
                if(!compare_options.top_input) {
                    eprintf_error("An input is required before setting a maps directory.");
                    std::exit(EXIT_FAILURE);
                }
                if(!top_option_is_map_input) {
                    eprintf_error("%s", CAN_ONLY_BE_USED_WITH_MAP_INPUT);
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
                if(!top_option_is_map_input) {
                    eprintf_error("%s", CAN_ONLY_BE_USED_WITH_TAG_INPUT);
                    std::exit(EXIT_FAILURE);
                }
                compare_options.top_input->tags.emplace_back(args[0]);
                break;
                
            case 'c': {
                auto class_to_check = extension_to_tag_class(args[0]);
                for(auto c : compare_options.class_to_check) {
                    if(c == class_to_check) {
                        eprintf_error("Class %s was already specified", args[0]);
                        std::exit(EXIT_FAILURE);
                    }
                }
                if(class_to_check == TagClassInt::TAG_CLASS_NULL || class_to_check == TagClassInt::TAG_CLASS_NONE) {
                    eprintf_error("Class %s is not a valid class", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                compare_options.class_to_check.push_back(class_to_check);
                break;
            }
                
            case 'p':
                compare_options.precision = true;
                break;
                
            case 'a':
                compare_options.match_all = true;
                break;
                
            case 's':
                if(std::strcmp(args[0], "all") == 0) {
                    compare_options.show = Show::SHOW_ALL;
                }
                else if(std::strcmp(args[0], "matched") == 0) {
                    compare_options.show = Show::SHOW_MATCHED;
                }
                else if(std::strcmp(args[0], "mismatched") == 0) {
                    compare_options.show = Show::SHOW_MISMATCHED;
                }
                else {
                    eprintf_error("%s is not a valid option for --show", args[0]);
                    std::exit(EXIT_FAILURE);
                }
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
    
    // Automatically make up maps directories for any map when necessary, then open their respective resources
    for(auto &i : compare_options.inputs) {
        if(i.map.has_value() && !i.maps.has_value()) {
            i.maps = std::filesystem::absolute(std::filesystem::path(*i.map)).parent_path();
        }
            
        if(i.map.has_value()) {
            // Load resource maps
            std::vector<std::byte> loc, bitmaps, sounds;
            if(i.maps.has_value() && !i.ignore_resource_maps) {
                loc = File::open_file((*i.maps / "loc.map").string().c_str()).value_or(std::vector<std::byte>());
                bitmaps = File::open_file((*i.maps / "bitmaps.map").string().c_str()).value_or(std::vector<std::byte>());
                sounds = File::open_file((*i.maps / "sounds.map").string().c_str()).value_or(std::vector<std::byte>());
            }
        
            auto data = File::open_file(*i.map);
            if(!data.has_value()) {
                eprintf_error("Failed to read %s", *i.map);
                return EXIT_FAILURE;
            }
            
            auto &map = *(i.map_data = std::make_unique<Map>(Map::map_with_move(*std::move(data),std::move(bitmaps),std::move(loc),std::move(sounds))));
            
            // Warn if we failed to open some resource maps
            if(!i.ignore_resource_maps) {
                switch(map.get_engine()) {
                    case HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION:
                        if(map.get_data_length(Invader::Map::DATA_MAP_LOC) == 0) {
                            eprintf_warn("Failed to find or read a loc.map");
                        }
                        // fallthrough
                    case HEK::CacheFileEngine::CACHE_FILE_DEMO:
                    case HEK::CacheFileEngine::CACHE_FILE_RETAIL:
                        if(map.get_data_length(Invader::Map::DATA_MAP_BITMAP) == 0) {
                            eprintf_warn("Failed to find or read a bitmaps.map");
                        }
                        if(map.get_data_length(Invader::Map::DATA_MAP_SOUND) == 0) {
                            eprintf_warn("Failed to find or read a sounds.map");
                        }
                        break;
                    default:
                        break;
                }
            }
            
            // Go through each tag and add them if we want to do the thing
            auto tag_count = map.get_tag_count();
            i.tag_paths.reserve(tag_count);
            for(std::size_t t = 0; t < tag_count; t++) {
                auto &tag = map.get_tag(t);
                auto tag_class_int = tag.get_tag_class_int();
                if(!tag.data_is_available() || std::strcmp(tag_class_to_extension(tag_class_int), "unknown") == 0) {
                    continue;
                }
                if(compare_options.class_to_check.size()) {
                    bool should_add = false;
                    for(auto c : compare_options.class_to_check) {
                        if(c == tag_class_int) {
                            should_add = true;
                            break;
                        }
                    }
                    if(!should_add) {
                        continue;
                    }
                }
                i.tag_paths.emplace_back(tag.get_path(), tag_class_int);
            }
        }
        else {
            try {
                i.virtual_directory = File::load_virtual_tag_folder(i.tags);
            }
            catch(std::exception &e) {
                eprintf_error("Failed to load the tags directory for an input: %s", e.what());
                return EXIT_FAILURE;
            }
            i.tag_paths.reserve(i.virtual_directory.size());
            for(auto &t : i.virtual_directory) {
                if(compare_options.class_to_check.size()) {
                    bool should_add = false;
                    for(auto c : compare_options.class_to_check) {
                        if(c == t.tag_class_int) {
                            should_add = true;
                            break;
                        }
                    }
                    if(!should_add) {
                        continue;
                    }
                }
                i.tag_paths.emplace_back(File::split_tag_class_extension(File::preferred_path_to_halo_path(t.tag_path)).value());
            }
        }
        i.tag_paths.shrink_to_fit();
    }
    
    regular_comparison(compare_options.inputs, compare_options.precision, compare_options.show, compare_options.match_all, compare_options.functional, compare_options.by_path);
}

static void regular_comparison(const std::vector<Input> &inputs, bool precision, Show show, bool match_all, bool functional, ByPath by_path) {
    // Find all tags we have in common first
    auto input_count = inputs.size();
    std::vector<File::TagFilePath> tags;
    
    #define CAN_COMPARE(by_path, path1, path2) ((by_path == ByPath::BY_PATH_SAME && path1 == path2) || (by_path == ByPath::BY_PATH_DIFFERENT && path1 != path2) || (by_path == ByPath::BY_PATH_ANY))
    
    // Do this thing
    if(match_all) {
        auto &first_input = inputs[0];
        auto input_count = inputs.size();
        tags.reserve(first_input.tag_paths.size());
        for(auto &tag : first_input.tag_paths) {
            bool not_found = false;
            for(std::size_t i = 1; i < input_count; i++) {
                bool found = false;
                for(auto &tag2 : inputs[i].tag_paths) {
                    if(tag2.class_int != tag.class_int) {
                        continue;
                    }
                    if(CAN_COMPARE(by_path, tag.path, tag2.path)) {
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    not_found = true;
                    break;
                }
            }
            if(not_found) {
                continue;
            }
            tags.push_back(tag);
        }
    }
    else {
        for(std::size_t i = 0; i < input_count; i++) {
            auto &input = inputs[i];
            for(std::size_t j = i + 1; j < input_count; j++) {
                auto &input2 = inputs[j];
                for(auto &tag : input.tag_paths) {
                    // Make sure we don't add any duplicates
                    bool found = false;
                    for(auto &otag : tags) {
                        if(otag == tag) {
                            found = true;
                            break;
                        }
                    }
                    if(found) {
                        continue;
                    }
                    
                    // Add it if it's present!
                    for(auto &tag2 : input2.tag_paths) {
                        if(tag2.class_int != tag.class_int) {
                            continue;
                        }
                        if(CAN_COMPARE(by_path, tag.path, tag2.path)) {
                            found = true;
                            tags.push_back(tag);
                            break;
                        }
                    }
                }
            }
        }
    }
    tags.shrink_to_fit();
    
    // Hold this for when we start outputting stuff
    bool show_all = (show & Show::SHOW_ALL) == Show::SHOW_ALL;
    
    // Next, compare each tag
    std::size_t matched_count = 0;
    std::size_t mismatched_count = 0;
    for(auto &tag : tags) {
        std::vector<std::unique_ptr<Parser::ParserStruct>> structs;
        std::vector<std::string> struct_paths;
        std::vector<const Input *> struct_inputs;
        
        bool first_input = true;
        bool only_finding_same_tag = true;
        
        // Go through each input
        for(auto &i : inputs) {
            // On the first input, we always break when we find the tag since we're only looking for tags with the same path to match the tag with the outer loop
            // On subsequent inputs, we only break if we're *always* looking for tags with the same path.
            auto by_path_copy = by_path;
            if(first_input) {
                first_input = false; // set to false
                by_path_copy = ByPath::BY_PATH_SAME;
            }
            
            only_finding_same_tag = by_path_copy == ByPath::BY_PATH_SAME;
            
            // If it's a map, do this
            if(i.map.has_value()) {
                // First, extract it
                auto tag_count = i.map_data->get_tag_count();
                for(std::size_t t = 0; t < tag_count; t++) {
                    auto &map_tag = i.map_data->get_tag(t);
                    auto &map_tag_path = map_tag.get_path();
                    if(map_tag.get_tag_class_int() == tag.class_int && CAN_COMPARE(by_path_copy, tag.path, map_tag_path)) {
                        auto extracted_data = Invader::ExtractionWorkload::extract_single_tag(i.map_data->get_tag(t));
                        structs.emplace_back(Parser::ParserStruct::parse_hek_tag_file(extracted_data.data(), extracted_data.size(), true));
                        struct_paths.emplace_back(map_tag_path);
                        struct_inputs.emplace_back(&i);
                            
                        if(only_finding_same_tag) {
                            break;
                        }
                    }
                }
            }
            
            // If it's a tag, do this
            else {
                for(auto &vd : i.virtual_directory) {
                    auto other_path = File::split_tag_class_extension(File::preferred_path_to_halo_path(vd.tag_path)).value().path;
                    if(vd.tag_class_int == tag.class_int && CAN_COMPARE(by_path_copy, tag.path, other_path)) {
                        // Open it
                        auto file = Invader::File::open_file(vd.full_path.string().c_str()).value();
                        
                        // Parse it
                        structs.emplace_back(Parser::ParserStruct::parse_hek_tag_file(file.data(), file.size(), true));
                        struct_paths.emplace_back(other_path);
                        struct_inputs.emplace_back(&i);
                        
                        if(only_finding_same_tag) {
                            break;
                        }
                    }
                }
            }
        }
        
        auto found_count = structs.size();
        if(found_count < 2) {
            continue;
        }
        
        #define MATCHED(type) "%s%s.%s", show_all ? type ": " : ""
        #define MATCHED_TO(type) "%s%s.%s, %s.%s", show_all ? type ": " : ""
        #define MATCHED_TO_DIFFERENT_INPUT(type) "%s%s.%s, %s.%s (%zu)", show_all ? type ": " : ""
        
        auto &first_struct = structs[0];
        
        // Just for setting counter/debugging
        auto match_log = [&tag, &matched_count, &show, &show_all, &mismatched_count, &struct_paths, &by_path, &struct_inputs, &inputs](bool did_match, std::size_t i) {
            auto *extension = HEK::tag_class_to_extension(tag.class_int);
            auto other_path = File::halo_path_to_preferred_path(struct_paths[i]);
            bool show_different_input = inputs.size() > 2; // only need to show differing inputs if we have more than two inputs
            std::size_t input_of_other = 1;
            
            // If we're using multiple inputs, get the input of the other thing
            if(show_different_input) {
                auto *other_input = struct_inputs[i];
                for(auto &i : inputs) {
                    if(&i == other_input) {
                        input_of_other = &i - inputs.data();
                        break;
                    }
                }
            }
            
            if(did_match) {
                if(show & Show::SHOW_MATCHED) {
                    if(by_path == ByPath::BY_PATH_SAME) {
                        oprintf_success(MATCHED("Matched"), File::halo_path_to_preferred_path(tag.path).c_str(), HEK::tag_class_to_extension(tag.class_int));
                    }
                    else if(show_different_input) {
                        oprintf_success(MATCHED_TO_DIFFERENT_INPUT("Matched"), File::halo_path_to_preferred_path(tag.path).c_str(), extension, other_path.c_str(), extension, input_of_other);
                    }
                    else {
                        oprintf_success(MATCHED_TO("Matched"), File::halo_path_to_preferred_path(tag.path).c_str(), extension, other_path.c_str(), extension);
                    }
                }
                matched_count++;
            }
            else {
                if(show & Show::SHOW_MISMATCHED) {
                    if(by_path == ByPath::BY_PATH_SAME) {
                        oprintf_success_warn(MATCHED("Mismatched"), File::halo_path_to_preferred_path(tag.path).c_str(), HEK::tag_class_to_extension(tag.class_int));
                    }
                    else if(show_different_input) {
                        oprintf_success_warn(MATCHED_TO_DIFFERENT_INPUT("Mismatched"), File::halo_path_to_preferred_path(tag.path).c_str(), extension, other_path.c_str(), extension, input_of_other);
                    }
                    else {
                        oprintf_success_warn(MATCHED_TO("Mismatched"), File::halo_path_to_preferred_path(tag.path).c_str(), extension, other_path.c_str(), extension);
                    }
                }
                mismatched_count++;
            }
        };
        
        if(functional) {
            try {
                auto meme_up_struct = [&tag](Parser::ParserStruct &struct_v) -> std::vector<std::uint8_t> {
                    auto hdata = struct_v.generate_hek_tag_data(tag.class_int);
                    std::vector<std::uint8_t> meme_data;
                    
                    // Compile it
                    auto compiled = BuildWorkload::compile_single_tag(hdata.data(), hdata.size());
                    
                    // Process each struct
                    for(auto &s : compiled.structs) {
                        // Process struct data
                        meme_data.insert(meme_data.end(), reinterpret_cast<const std::uint8_t *>(s.data.data()), reinterpret_cast<const std::uint8_t *>(s.data.data() + s.data.size()));
                        
                        // Process each dependency
                        for(auto &d : s.dependencies) {
                            char o[1024] = {};
                            auto len = std::snprintf(o, sizeof(o), "D:%08zX->%08zX!", d.offset, d.tag_index);
                            meme_data.insert(meme_data.end(), o, o + len);
                        }
                        
                        // Process each pointer
                        for(auto &p : s.pointers) {
                            char o[1024] = {};
                            auto len = std::snprintf(o, sizeof(o), "P:%08zX->%08zX!", p.offset, p.struct_index);
                            meme_data.insert(meme_data.end(), o, o + len);
                        }
                    }
                    
                    // Process each tag
                    for(auto &t : compiled.tags) {
                        char o[1024] = {};
                        auto len = std::snprintf(o, sizeof(o), "T:%s.%s!", t.path.c_str(), HEK::tag_class_to_extension(t.tag_class_int));
                        meme_data.insert(meme_data.end(), o, o + len);
                    }
                    
                    return meme_data;
                };
                
                auto first_meme = meme_up_struct(*first_struct);
                for(std::size_t i = 1; i < found_count; i++) {
                    auto mms = meme_up_struct(*structs[i]);
                    match_log(first_meme == mms, i);
                }
            }
            catch(std::exception &e) {
                eprintf_error("Cannot functional compare %s.%s due to an error: %s", File::halo_path_to_preferred_path(tag.path).c_str(), HEK::tag_class_to_extension(tag.class_int), e.what());
            }
        }
        else {
            for(std::size_t i = 1; i < found_count; i++) {
                match_log(first_struct->compare(structs[i].get(), precision, true), i);
            }
        }
    }
    
    // Show the total matched if we are showing both
    if(show_all) {
        auto total = matched_count + mismatched_count;
        oprintf("Matched %zu / %zu tag%s\n", matched_count, total, total == 1 ? "" : "s");
    }
}
