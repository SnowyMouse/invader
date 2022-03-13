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
#include <thread>
#include <mutex>

#include "bludgeoner.hpp"

using namespace Invader;
using namespace Invader::Bludgeoner;

struct BludgeonAction {
    // Simple counter that increments every time it's initialized
    struct FixBit {
        std::uint64_t value;
        
        operator std::uint64_t() const noexcept {
            return this->value;
        }
        
        FixBit() {
            static std::uint64_t fix_bit_current = 1;
            this->value = fix_bit_current;
            fix_bit_current <<= 1;
        }
        
        FixBit(std::uint64_t value) {
            this->value = value;
        }
    };
    
    const char *name;
    std::optional<bool (*)(Parser::ParserStruct *s, bool fix)> fix_fn = std::nullopt;
    FixBit fix_bit = FixBit();
};

static BludgeonAction all_fixes[] = {
    /** Fix broken rotation function scales (caused by extracting maps made with older tools that broke this value, even the original HEK) */
    { .name = "broken-lens-flare-function-scale", .fix_fn = broken_lens_flare_function_scale },
    
    /** Fix an incorrect sound buffer size */
    { .name = "incorrect-sound-buffer", .fix_fn = sound_buffer },

    /** Fix invalid enums being used (this is present in stock tags, and in some cases with HEK+ extracted tags,
        this has led to the game crashing) */
    { .name = "invalid-enums", .fix_fn = broken_enums },
    
    /** Fix indices that are out of bounds */
    { .name = "invalid-indices", .fix_fn = broken_indices_fix },

    /** Fix strings not being null terminated or being the wrong length */
    { .name = "invalid-strings", .fix_fn = broken_strings },
    
    /** Fix model markers not being put in the right place (not having this results in undefined behavior when built by
        tool.exe) */
    { .name = "invalid-model-markers", .fix_fn = invalid_model_markers },
    
    /** Fix bullshit tag references being used (e.g. light tags being referenced in models) */
    { .name = "invalid-reference-classes", .fix_fn = broken_references },
    
    /** Fix uppercase references */
    { .name = "invalid-uppercase-references", .fix_fn = uppercase_references },
    
    /** Fix mismatched sound enums */
    { .name = "mismatched-sound-enums", .fix_fn = mismatched_sound_enums },

    /** Extract scripts (not having this results in undefined behavior when built by tool.exe) */
    { .name = "missing-script-source", .fix_fn = missing_scripts },

    /** Regenerate missing compressed/uncompressed vertices (not having these fucks up lightmap generation) */
    { .name = "missing-vertices", .fix_fn = broken_vertices },

    /** Fix normals (broken normals crashes tool.exe and sapien when generating lightmaps) */
    { .name = "nonnormal-vectors", .fix_fn = broken_normals },
    
    /** Fix invalid values that are out of bounds for their ranges */
    { .name = "out-of-range", .fix_fn = broken_range_fix },
    
    /** Fix everything */
    { .name = "everything", .fix_bit = static_cast<std::uint64_t>(~0) }
};

// Singleton the printf!
static std::mutex bad_code_design_mutex;
#define badly_designed_printf(function, ...) bad_code_design_mutex.lock(); \
                                             function(__VA_ARGS__); \
                                             bad_code_design_mutex.unlock();

static int bludgeon_tag(const std::filesystem::path &file_path, const std::string &tag_path, std::uint64_t fixes, bool &bludgeoned) {
    using namespace Bludgeoner;
    using namespace HEK;
    using namespace File;

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
        HEK::TagFileHeader::validate_header(header, tag->size());
        auto parsed_data = Parser::ParserStruct::parse_hek_tag_file(tag->data(), tag->size());

        // No fixes; try to detect things
        bool issues_present = false;
        if(fixes == 0) {
            for(auto &i : all_fixes) {
                if(i.fix_fn.has_value()) {
                    if((*i.fix_fn)(parsed_data.get(), false)) {
                        badly_designed_printf(oprintf_success_warn, "%s: Detected %s", tag_path.c_str(), i.name);
                        issues_present = true;
                    }
                }
            }
        }
        else {
            for(auto &i : all_fixes) {
                if(i.fix_fn.has_value() && (i.fix_bit & fixes) != 0) {
                    if((*i.fix_fn)(parsed_data.get(), true)) {
                        badly_designed_printf(oprintf_success, "%s: Fixed %s", tag_path.c_str(), i.name);
                        issues_present = true;
                    }
                }
            }
        }

        // No issues? OK
        if(!issues_present) {
            return EXIT_SUCCESS;
        }
        
        bludgeoned = true;

        // Exit out of here
        if(fixes == 0) {
            return EXIT_SUCCESS;
        }

        // Do it!
        file_data = parsed_data->generate_hek_tag_data(header->tag_fourcc, true);
        if(!File::save_file(file_path, file_data)) {
            badly_designed_printf(eprintf_error, "Error: Failed to write to %s.", file_path.string().c_str());
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    catch(std::exception &e) {
        badly_designed_printf(eprintf_error, "Error: Failed to bludgeon %s: %s", tag_path.c_str(), e.what());
        return EXIT_FAILURE;
    }
}

int main(int argc, char * const *argv) {
    set_up_color_term();
    
    std::string issues_list;
    for(auto &i : all_fixes) {
        if(!issues_list.empty()) {
            issues_list += ", ";
        }
        issues_list += i.name;
    }
    issues_list = std::string("Type of bludgeoning. Can be: ") + issues_list;
    
    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS),
        CommandLineOption("threads", 'j', 1, "Set the number of threads to use for parallel bludgeoning when using --all. Default: CPU thread count"),
        CommandLineOption("search", 's', 1, "Search for tags (* and ? are wildcards) and bludgeon these. Use multiple times for multiple queries. If unspecified, all tags will be bludgeoned.", "<expr>"),
        CommandLineOption("search-exclude", 'e', 1, "Search for tags (* and ? are wildcards) and ignore these. Use multiple times for multiple queries. This takes precedence over --search.", "<expr>"),
        CommandLineOption("type", 'T', 1, issues_list.c_str())
    };

    static constexpr char DESCRIPTION[] = "Convinces tags to work with Invader.";
    static constexpr char USAGE[] = "[options]";

    struct BludgeonOptions {
        std::optional<std::filesystem::path> tags;
        std::uint64_t fixes = 0;
        std::vector<std::string> search;
        std::vector<std::string> search_exclude;
        std::size_t max_threads = std::thread::hardware_concurrency() < 1 ? 1 : std::thread::hardware_concurrency();
    } bludgeon_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<BludgeonOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, bludgeon_options, [](char opt, const std::vector<const char *> &arguments, auto &bludgeon_options) {
        using namespace Invader;
        switch(opt) {
            case 't':
                if(bludgeon_options.tags.has_value()) {
                    eprintf_error("This tool does not support multiple tags directories.");
                    std::exit(EXIT_FAILURE);
                }
                bludgeon_options.tags = arguments[0];
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                
            case 's':
                bludgeon_options.search.emplace_back(File::preferred_path_to_halo_path(arguments[0]));
                break;
                
            case 'e':
                bludgeon_options.search_exclude.emplace_back(File::preferred_path_to_halo_path(arguments[0]));
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
            case 'T':
                for(auto &i : all_fixes) {
                    if(std::strcmp(arguments[0], i.name) == 0) {
                        bludgeon_options.fixes |= i.fix_bit;
                        return;
                    }
                }
                eprintf_error("Unknown fix type %s", arguments[0]);
                std::exit(EXIT_FAILURE);
                break;
        }
    });
    
    if(!bludgeon_options.tags.has_value()) {
        bludgeon_options.tags = "tags";
    }

    auto &fixes = bludgeon_options.fixes;

    std::size_t success = 0;
    
    auto all_virtual_tags = File::load_virtual_tag_folder(std::vector<std::filesystem::path>(&*bludgeon_options.tags, &*bludgeon_options.tags + 1));
    decltype(all_virtual_tags) all_tags;
    all_tags.reserve(all_virtual_tags.size());
    
    if(bludgeon_options.search.empty() && bludgeon_options.search_exclude.empty()) {
        all_tags = std::move(all_virtual_tags);
    }
    else {
        for(auto &i : all_virtual_tags) {
            if(File::path_matches(i.tag_path.c_str(), bludgeon_options.search, bludgeon_options.search_exclude)) {
                all_tags.emplace_back(std::move(i));
            }
        }
    }
    
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
            auto &tag = all_tags->data()[this_index];
            bludgeon_tag(tag.full_path, tag.tag_path, *fixes, bludgeoned);
            
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
