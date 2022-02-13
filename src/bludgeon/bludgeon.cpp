// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/command_line_option.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>
#include <thread>
#include <mutex>

#include "bludgeoner.hpp"

enum WaysToFuckUpTheTag : std::uint64_t {
    /** Apply no fixes; just see what we can do */
    NO_FIXES                            = 0,

    /** Fix color change in tag being incorrect */
    INVALID_COLOR_CHANGE                = 1ull << 0,

    /** Fix invalid enums being used (this is present in stock HEK tags, and in some cases with HEK+ extracted tags,
        this has led to the game crashing) */
    BROKEN_ENUMS                        = 1ull << 1,

    /** Fix bullshit tag references being used (e.g. light tags being referenced in models) */
    BROKEN_REFERENCE_CLASSES            = 1ull << 2,

    /** Fix the incorrect sound format being reported in the header (incorrect format was done to work around Halo not
        allowing 16-bit PCM, but this was fixed in mods, and this is undefined behavior) */
    BROKEN_SOUND_FORMAT                 = 1ull << 3,

    /** Fix strings not being null terminated or being the wrong length */
    BROKEN_STRINGS                      = 1ull << 4,

    /** Extract scripts (not having this results in undefined behavior when built by tool.exe) */
    MISSING_SCRIPTS                     = 1ull << 5,

    /** Fix an incorrect sound buffer size */
    INVALID_SOUND_BUFFER                = 1ull << 6,
    
    /** Fix invalid values that are out of bounds for their ranges */
    BROKEN_RANGE                        = 1ull << 7,
    
    /** Fix indices that are out of bounds */
    INVALID_INDICES                     = 1ull << 8,

    /** Fix normals (broken normals crashes tool.exe and sapien when generating lightmaps) */
    INVALID_NORMALS                     = 1ull << 9,
    
    /** Fix model markers not being put in the right place (not having this results in undefined behavior when built by
        tool.exe) */
    INVALID_MODEL_MARKERS               = 1ull << 10,

    /** Regenerate missing compressed/uncompressed vertices (not having these fucks up lightmap generation) */
    INVALID_VERTICES                    = 1ull << 11,

    /** Fix sound permutations not being valid (caused by old versions of Refinery when safe mode is enabled; this cannot be fixed, but it can at least be turned into something technically valid) */
    INVALID_SOUND_PERMUTATIONS          = 1ull << 12,
    
    /** Fix uppercase references */
    INVALID_UPPERCASE_REFERENCES        = 1ull << 13,
    
    /** Fix broken rotation function scales (caused by extracting maps made with older tools that broke this value, even the original HEK) */
    BROKEN_LENS_FLARE_ROTATION_SCALE    = 1ull << 14,
    
    /** Attempt to unfuck anything that can be unfucked (CAUTION: you can unscrew a lightbulb; you can't unscrew a Halo tag) */
    EVERYTHING                          = ~0ull
};

#define NO_FIXES_FIX "none"
#define INVALID_COLOR_CHANGE_FIX "broken-color-change"
#define BROKEN_ENUMS_FIX "invalid-enums"
#define BROKEN_REFERENCE_CLASSES_FIX "invalid-reference-classes"
#define INVALID_UPPERCASE_REFERENCES_FIX "invalid-uppercase-references"
#define BROKEN_SOUND_FORMAT_FIX "invalid-sound-format"
#define INVALID_MODEL_MARKERS_FIX "invalid-model-markers"
#define INVALID_VERTICES_FIX "missing-vertices"
#define MISSING_SCRIPTS_FIX "missing-script-source"
#define BROKEN_RANGE_FIX "out-of-range"
#define INVALID_SOUND_PERMUTATIONS_FIX "invalid-sound-permutations"
#define INVALID_SOUND_BUFFER_FIX "incorrect-sound-buffer"
#define INVALID_INDICES_FIX "invalid-indices"
#define INVALID_NORMALS_FIX "nonnormal-vectors"
#define BROKEN_STRINGS_FIX "invalid-strings"
#define BROKEN_LENS_FLARE_ROTATION_SCALE_FIX "broken-lens-flare-function-scale"
#define EVERYTHING_FIX "everything"

// Singleton the printf!
static std::mutex bad_code_design_mutex;
#define badly_designed_printf(function, ...) bad_code_design_mutex.lock(); \
                                             function(__VA_ARGS__); \
                                             bad_code_design_mutex.unlock();

static int bludgeon_tag(const std::filesystem::path &file_path, std::uint64_t fixes, bool &bludgeoned) {
    using namespace Invader::Bludgeoner;
    using namespace Invader::HEK;
    using namespace Invader::File;

    bludgeoned = false;

    // Open the tag
    auto tag = open_file(file_path);
    if(!tag.has_value()) {
        badly_designed_printf(eprintf_error, "Failed to open %s", file_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Get the header
    std::vector<std::byte> file_data;
    try {
        const auto *header = reinterpret_cast<const TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());
        auto parsed_data = Invader::Parser::ParserStruct::parse_hek_tag_file(tag->data(), tag->size());

        // No fixes; try to detect things
        bool issues_present = false;
        if(fixes == WaysToFuckUpTheTag::NO_FIXES) {
            #define check_fix(fix, fix_message) if(fix(parsed_data.get(), false)) { \
                badly_designed_printf(oprintf_success_warn, "%s: " fix_message, file_path.string().c_str()); \
                issues_present = true; \
            }
            
            check_fix(broken_enums, "invalid enums detected; fix with " BROKEN_ENUMS_FIX);
            check_fix(broken_references, "invalid reference class detected; fix with " BROKEN_REFERENCE_CLASSES_FIX);
            check_fix(invalid_model_markers, "invalid model markers detected; fix with " INVALID_MODEL_MARKERS_FIX);
            check_fix(sound_buffer, "incorrect sound buffer size on one or more permutations; fix with " INVALID_SOUND_BUFFER_FIX);
            check_fix(broken_vertices, "missing compressed or uncompressed vertices; fix with " INVALID_VERTICES_FIX);
            check_fix(broken_range_fix, "value(s) are out of range; fix with " BROKEN_RANGE_FIX);
            check_fix(missing_scripts, "script source data is missing; fix with " MISSING_SCRIPTS_FIX);
            check_fix(broken_indices_fix, "indices are out of bounds; fix with " INVALID_INDICES_FIX);
            check_fix(broken_normals, "problematic nonnormal vectors detected; fix with " INVALID_NORMALS_FIX);
            check_fix(broken_strings, "problematic strings detected; fix with " BROKEN_STRINGS_FIX);
            check_fix(uppercase_references, "uppercase references detected; fix with " INVALID_UPPERCASE_REFERENCES_FIX);
            check_fix(broken_lens_flare_function_scale, "broken lens flare function scale; fix with " BROKEN_LENS_FLARE_ROTATION_SCALE_FIX);
            
            #undef check_fix
        }
        else {
            #define apply_fix(fix, fix_enum, fix_name) if((fixes & fix_enum) && fix(parsed_data.get(), true)) { \
                badly_designed_printf(oprintf_success, "%s: Fixed " fix_name, file_path.string().c_str()); \
                issues_present = true; \
            }
            
            apply_fix(broken_enums, BROKEN_ENUMS, BROKEN_ENUMS_FIX);
            apply_fix(broken_references, BROKEN_REFERENCE_CLASSES, BROKEN_REFERENCE_CLASSES_FIX);
            apply_fix(invalid_model_markers, INVALID_MODEL_MARKERS, INVALID_MODEL_MARKERS_FIX);
            apply_fix(broken_range_fix, BROKEN_RANGE, BROKEN_RANGE_FIX);
            apply_fix(sound_buffer, INVALID_SOUND_BUFFER, INVALID_SOUND_BUFFER_FIX);
            apply_fix(broken_vertices, INVALID_VERTICES, INVALID_VERTICES_FIX);
            apply_fix(missing_scripts, MISSING_SCRIPTS, MISSING_SCRIPTS_FIX);
            apply_fix(broken_indices_fix, INVALID_INDICES, INVALID_INDICES_FIX);
            apply_fix(broken_normals, INVALID_NORMALS, INVALID_NORMALS_FIX);
            apply_fix(broken_strings, BROKEN_STRINGS, BROKEN_STRINGS_FIX);
            apply_fix(uppercase_references, INVALID_UPPERCASE_REFERENCES, INVALID_UPPERCASE_REFERENCES_FIX);
            apply_fix(broken_lens_flare_function_scale, BROKEN_LENS_FLARE_ROTATION_SCALE, BROKEN_LENS_FLARE_ROTATION_SCALE_FIX);
            
            #undef apply_fix
        }

        // No issues? OK
        if(!issues_present) {
            return EXIT_SUCCESS;
        }
        
        bludgeoned = true;

        // Exit out of here
        if(fixes == WaysToFuckUpTheTag::NO_FIXES) {
            return EXIT_SUCCESS;
        }

        // Do it!
        file_data = parsed_data->generate_hek_tag_data(header->tag_fourcc, true);
        if(!Invader::File::save_file(file_path, file_data)) {
            badly_designed_printf(eprintf_error, "Error: Failed to write to %s.", file_path.string().c_str());
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    catch(std::exception &e) {
        badly_designed_printf(eprintf_error, "Error: Failed to bludgeon %s: %s", file_path.string().c_str(), e.what());
        return EXIT_FAILURE;
    }
}

int main(int argc, char * const *argv) {
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path if specifying a tag.");
    options.emplace_back("all", 'a', 0, "Bludgeon all tags in the tags directory.");
    options.emplace_back("threads", 'j', 1, "Set the number of threads to use for parallel bludgeoning when using --all. Default: CPU thread count");
    options.emplace_back("type", 'T', 1, "Type of bludgeoning. Can be: " BROKEN_ENUMS_FIX ", " BROKEN_RANGE_FIX ", " BROKEN_STRINGS_FIX ", " BROKEN_REFERENCE_CLASSES_FIX ", " INVALID_MODEL_MARKERS_FIX ", " MISSING_SCRIPTS_FIX ", " INVALID_SOUND_BUFFER_FIX ", " INVALID_VERTICES_FIX ", " INVALID_NORMALS_FIX ", " INVALID_UPPERCASE_REFERENCES_FIX ", " BROKEN_LENS_FLARE_ROTATION_SCALE_FIX ", " NO_FIXES_FIX ", " EVERYTHING_FIX " (default: " NO_FIXES_FIX ")");

    static constexpr char DESCRIPTION[] = "Convinces tags to work with Invader.";
    static constexpr char USAGE[] = "[options] <-a | tag.class>";

    struct BludgeonOptions {
        std::optional<std::filesystem::path> tags;
        bool use_filesystem_path = false;
        bool all = false;
        std::uint64_t fixes = WaysToFuckUpTheTag::NO_FIXES;
        std::size_t max_threads = std::thread::hardware_concurrency() < 1 ? 1 : std::thread::hardware_concurrency();
    } bludgeon_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<BludgeonOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 1, bludgeon_options, [](char opt, const std::vector<const char *> &arguments, auto &bludgeon_options) {
        switch(opt) {
            case 't':
                if(bludgeon_options.tags.has_value()) {
                    eprintf_error("This tool does not support multiple tags directories.");
                    std::exit(EXIT_FAILURE);
                }
                bludgeon_options.tags = arguments[0];
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'P':
                bludgeon_options.use_filesystem_path = true;
                break;
            case 'a':
                bludgeon_options.all = true;
                break;
            case 'j':
                try {
                    bludgeon_options.max_threads = std::stoi(arguments[0]);
                    if(bludgeon_options.max_threads < 1) {
                        throw std::exception();
                    }
                }
                catch(std::exception &) {
                    eprintf_error("Invalid number of threads %s\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
                break;
            case 'T':
                if(std::strcmp(arguments[0], NO_FIXES_FIX) == 0) {
                    bludgeon_options.fixes = WaysToFuckUpTheTag::NO_FIXES;
                }
                else if(std::strcmp(arguments[0], INVALID_COLOR_CHANGE_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_COLOR_CHANGE;
                }
                else if(std::strcmp(arguments[0], BROKEN_ENUMS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BROKEN_ENUMS;
                }
                else if(std::strcmp(arguments[0], BROKEN_REFERENCE_CLASSES_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BROKEN_REFERENCE_CLASSES;
                }
                else if(std::strcmp(arguments[0], MISSING_SCRIPTS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::MISSING_SCRIPTS;
                }
                else if(std::strcmp(arguments[0], BROKEN_SOUND_FORMAT_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BROKEN_SOUND_FORMAT;
                }
                else if(std::strcmp(arguments[0], INVALID_MODEL_MARKERS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_MODEL_MARKERS;
                }
                else if(std::strcmp(arguments[0], INVALID_VERTICES_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_VERTICES;
                }
                else if(std::strcmp(arguments[0], INVALID_SOUND_PERMUTATIONS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_SOUND_PERMUTATIONS;
                }
                else if(std::strcmp(arguments[0], INVALID_SOUND_BUFFER_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_SOUND_BUFFER;
                }
                else if(std::strcmp(arguments[0], BROKEN_RANGE_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BROKEN_RANGE;
                }
                else if(std::strcmp(arguments[0], INVALID_INDICES_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_INDICES;
                }
                else if(std::strcmp(arguments[0], INVALID_NORMALS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_NORMALS;
                }
                else if(std::strcmp(arguments[0], BROKEN_STRINGS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BROKEN_STRINGS;
                }
                else if(std::strcmp(arguments[0], INVALID_UPPERCASE_REFERENCES_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::INVALID_UPPERCASE_REFERENCES;
                }
                else if(std::strcmp(arguments[0], BROKEN_LENS_FLARE_ROTATION_SCALE_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BROKEN_LENS_FLARE_ROTATION_SCALE;
                }
                else if(std::strcmp(arguments[0], EVERYTHING_FIX) == 0) {
                    bludgeon_options.fixes = WaysToFuckUpTheTag::EVERYTHING;
                }
                else {
                    eprintf_error("Unknown fix type %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });
    
    if(!bludgeon_options.tags.has_value()) {
        bludgeon_options.tags = "tags";
    }

    auto &fixes = bludgeon_options.fixes;

    if(remaining_arguments.size() == 0) {
        if(!bludgeon_options.all) {
            eprintf_error("Expected --all to be used OR a tag path. Use -h for more information.");
            return EXIT_FAILURE;
        }

        std::size_t success = 0;
        
        auto all_tags = Invader::File::load_virtual_tag_folder(std::vector<std::filesystem::path>(&*bludgeon_options.tags, &*bludgeon_options.tags + 1));
        std::mutex thread_mutex;
        std::vector<std::thread> threads;
        std::size_t tag_index = 0;
        threads.reserve(bludgeon_options.max_threads);
        
        auto bludgeon_worker = [](auto *all_tags, std::size_t *tag_index, std::mutex *thread_mutex, std::size_t *success, auto *fixes) {
            while(true) {
                thread_mutex->lock();
                std::size_t this_index = *tag_index;
                if(this_index == all_tags->size()) {
                    thread_mutex->unlock();
                    return;
                }
                (*tag_index)++;
                thread_mutex->unlock();
                
                // Bludgeon
                bool bludgeoned;
                bludgeon_tag(all_tags->data()[this_index].full_path, *fixes, bludgeoned);
                
                // Increment
                thread_mutex->lock();
                (*success) += bludgeoned;
                thread_mutex->unlock();
            }
        };
        
        // Go through each tag
        for(std::size_t i = 0; i < bludgeon_options.max_threads; i++) {
            threads.emplace_back(bludgeon_worker, &all_tags, &tag_index, &thread_mutex, &success, &fixes);
        }
        
        // Wait for all threads to end
        for(auto &i : threads) {
            i.join();
        }

        oprintf("%s %zu out of %zu tag%s\n", fixes ? "Bludgeoned" : "Identified issues with", success, tag_index, tag_index == 1 ? "" : "s");

        return EXIT_SUCCESS;
    }
    else if(bludgeon_options.all) {
        eprintf_error("--all and a tag path cannot be used at the same time. Use -h for more information.");
        return EXIT_FAILURE;
    }
    else {
        std::filesystem::path file_path;
        if(bludgeon_options.use_filesystem_path) {
            file_path = std::string(remaining_arguments[0]);
        }
        else {
            file_path = std::filesystem::path(*bludgeon_options.tags) / Invader::File::halo_path_to_preferred_path(remaining_arguments[0]);
        }
        std::string file_path_str = file_path.string();
        bool bludgeoned;
        int result = bludgeon_tag(file_path_str.c_str(), fixes, bludgeoned);
        if(result == EXIT_SUCCESS && !bludgeoned) {
            oprintf("%s: No issues detected\n", file_path_str.c_str());
        }
        return result;
    }
}
