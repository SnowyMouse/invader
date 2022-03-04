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
    std::optional<bool (*)(Invader::Parser::ParserStruct *s, bool fix)> fix_fn;
    FixBit fix_bit;
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
        if(fixes == 0) {
            for(auto &i : all_fixes) {
                if(i.fix_fn.has_value()) {
                    if((*i.fix_fn)(parsed_data.get(), false)) {
                        badly_designed_printf(oprintf_success_warn, "%s: Detected %s", file_path.string().c_str(), i.name);
                        issues_present = true;
                    }
                }
            }
        }
        else {
            for(auto &i : all_fixes) {
                if(i.fix_fn.has_value() && (i.fix_bit & fixes) != 0) {
                    if((*i.fix_fn)(parsed_data.get(), true)) {
                        badly_designed_printf(oprintf_success, "%s: Fixed %s", file_path.string().c_str(), i.name);
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
    set_up_color_term();
    
    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path if specifying a tag.");
    options.emplace_back("all", 'a', 0, "Bludgeon all tags in the tags directory.");
    options.emplace_back("threads", 'j', 1, "Set the number of threads to use for parallel bludgeoning when using --all. Default: CPU thread count");
    
    std::string issues_list;
    for(auto &i : all_fixes) {
        if(!issues_list.empty()) {
            issues_list += ", ";
        }
        issues_list += i.name;
    }
    issues_list = std::string("Type of bludgeoning. Can be: ") + issues_list;
    
    options.emplace_back("type", 'T', 1, issues_list.c_str());

    static constexpr char DESCRIPTION[] = "Convinces tags to work with Invader.";
    static constexpr char USAGE[] = "[options] <-a | tag.class>";

    struct BludgeonOptions {
        std::optional<std::filesystem::path> tags;
        bool use_filesystem_path = false;
        bool all = false;
        std::uint64_t fixes = 0;
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
