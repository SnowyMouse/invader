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

enum WaysToFuckUpTheTag : std::uint64_t {
    /** Apply no fixes; just see what we can do */
    NO_FIXES                            = 0,

    /** Fix color change in tag being incorrect */
    FUCKED_COLOR_CHANGE                 = 1ull << 0,

    /** Fix invalid enums being used (this is present in stock HEK tags, and in some cases with HEK+ extracted tags,
        this has led to the game crashing) */
    BULLSHIT_ENUMS                      = 1ull << 1,

    /** Fix bullshit tag references being used (e.g. light tags being referenced in models) */
    BULLSHIT_ESCHATON_REFERENCES        = 1ull << 2,

    /** Fix the incorrect sound format being reported in the header (incorrect format was done to work around Halo not
        allowing 16-bit PCM, but this was fixed in mods, and this is undefined behavior) */
    BULLSHIT_SOUND_FORMAT               = 1ull << 3,

    /** Fix non-power-of-two tags being marked as power-of-two and vice versa (incorrect Po2 flag was done to work
        around Sapien, but it causes a myriad of issues and undefined behavior as a result) */
    POWER_OF_FUCK_YOU                   = 1ull << 4,

    /** Extract scripts (not having this results in undefined behavior when built by tool.exe) */
    // WHERE_THE_FUCK_ARE_THE_SCRIPTS      = 1ull << 5, // disabled until we can figure out script decompilation

    // Stuff that Refinery breaks

    /** Fix model markers not being put in the right place (not having this results in undefined behavior when built by
        tool.exe) */
    REFINERY_MODEL_MARKERS              = 1ull << 61,

    /** Regenerate missing compressed/uncompressed vertices (not having these fucks up lightmap generation - why the
        fuck was this made off by default????) */
    REFINERY_VERTICES                   = 1ull << 62,

    /** Make sound tags that were truncated by Refinery' "safe mode" bullshit valid again (basically Refinery turns
        perfectly valid tags into invalid tags BY DEFAULT; this is a longstanding issue that's been ignored - see
        https://github.com/Sigmmma/refinery/issues/13) */
    REFINERY_SOUND_PERMUTATIONS         = 1ull << 63,

    /** Attempt to unfuck anything that can be unfucked (you can unscrew a lightbulb; you can't unscrew a Halo tag) */
    EVERYTHING                          = ~0ull
};

#define NO_FIXES_FIX "none"
#define FUCKED_COLOR_CHANGE_FIX "broken-color-change"
#define BULLSHIT_ENUMS_FIX "invalid-enums"
#define BULLSHIT_ESCHATON_REFERENCES_FIX "invalid-references"
#define BULLSHIT_SOUND_FORMAT_FIX "invalid-sound-format"
#define POWER_OF_FUCK_YOU_FIX "invalid-power-of-two"
#define REFINERY_MODEL_MARKERS_FIX "invalid-model-markers"
#define REFINERY_VERTICES_FIX "invalid-vertices"
#define REFINERY_SOUND_PERMUTATIONS_FIX "invalid-sound-permutations"
#define EVERYTHING_FIX "everything"

static bool bullshit_enums(Invader::Parser::ParserStruct *s, bool fix) {
    return s->check_for_broken_enums(fix);
}

static int bludgeon_tag(const char *file_path, std::uint64_t fixes) {
    // Open the tag
    auto tag = Invader::File::open_file(file_path);
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path);
        return EXIT_FAILURE;
    }

    // Get the header
    std::vector<std::byte> file_data;
    try {
        const auto *header = reinterpret_cast<const Invader::HEK::TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());
        auto parsed_data = Invader::Parser::ParserStruct::parse_hek_tag_file(tag->data(), tag->size());

        // No fixes; try to detect things
        bool issues_present = false;
        if(fixes == WaysToFuckUpTheTag::NO_FIXES) {
            if(bullshit_enums(parsed_data.get(), false)) {
                oprintf_success_warn("%s: invalid enums detected; fix with " BULLSHIT_ENUMS_FIX, file_path);
                issues_present = true;
            }
        }
        else {
            // Detect bullshit enums
            if((fixes & BULLSHIT_ENUMS) && bullshit_enums(parsed_data.get(), true)) {
                oprintf_success("%s: Fixed %s", file_path, BULLSHIT_ENUMS_FIX);
                issues_present = true;
            }
        }

        // No issues? OK
        if(!issues_present) {
            oprintf("%s: No issues detected\n", file_path);
            return EXIT_SUCCESS;
        }

        // Exit out of here
        if(fixes == WaysToFuckUpTheTag::NO_FIXES) {
            return EXIT_SUCCESS;
        }

        file_data = parsed_data->generate_hek_tag_data(header->tag_class_int, true);
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to bludgeon %s: %s", file_path, e.what());
        return EXIT_FAILURE;
    }

    if(!Invader::File::save_file(file_path, file_data)) {
        eprintf_error("Error: Failed to write to %s.", file_path);
        return EXIT_FAILURE;
    }

    oprintf_success("Bludgeoned %s", file_path);

    return EXIT_SUCCESS;
}

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory.", "<dir>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path if specifying a tag.");
    options.emplace_back("all", 'a', 0, "Bludgeon all tags in the tags directory.");
    options.emplace_back("type", 'T', 1, "Type of bludgeoning. Can be: " BULLSHIT_ENUMS_FIX ", " NO_FIXES_FIX ", " EVERYTHING_FIX " (default: " NO_FIXES_FIX ")");

    static constexpr char DESCRIPTION[] = "Convinces tags to work with Invader.";
    static constexpr char USAGE[] = "[options] <-a | tag.class>";

    struct BludgeonOptions {
        const char *tags = "tags";
        bool use_filesystem_path = false;
        bool all = false;
        std::uint64_t fixes = WaysToFuckUpTheTag::NO_FIXES;
    } bludgeon_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<BludgeonOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 1, bludgeon_options, [](char opt, const std::vector<const char *> &arguments, auto &bludgeon_options) {
        switch(opt) {
            case 't':
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
            case 'T':
                if(std::strcmp(arguments[0], NO_FIXES_FIX) == 0) {
                    bludgeon_options.fixes = WaysToFuckUpTheTag::NO_FIXES;
                }
                else if(std::strcmp(arguments[0], FUCKED_COLOR_CHANGE_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::FUCKED_COLOR_CHANGE;
                }
                else if(std::strcmp(arguments[0], BULLSHIT_ENUMS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BULLSHIT_ENUMS;
                }
                else if(std::strcmp(arguments[0], BULLSHIT_ESCHATON_REFERENCES_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BULLSHIT_ESCHATON_REFERENCES;
                }
                else if(std::strcmp(arguments[0], BULLSHIT_SOUND_FORMAT_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::BULLSHIT_SOUND_FORMAT;
                }
                else if(std::strcmp(arguments[0], POWER_OF_FUCK_YOU_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::POWER_OF_FUCK_YOU;
                }
                else if(std::strcmp(arguments[0], REFINERY_MODEL_MARKERS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::REFINERY_MODEL_MARKERS;
                }
                else if(std::strcmp(arguments[0], REFINERY_MODEL_MARKERS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::REFINERY_MODEL_MARKERS;
                }
                else if(std::strcmp(arguments[0], REFINERY_VERTICES_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::REFINERY_VERTICES;
                }
                else if(std::strcmp(arguments[0], REFINERY_SOUND_PERMUTATIONS_FIX) == 0) {
                    bludgeon_options.fixes = bludgeon_options.fixes | WaysToFuckUpTheTag::REFINERY_SOUND_PERMUTATIONS;
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

    auto &fixes = bludgeon_options.fixes;

    if(remaining_arguments.size() == 0) {
        if(!bludgeon_options.all) {
            eprintf_error("Expected --all to be used OR a tag path. Use -h for more information.");
            return EXIT_FAILURE;
        }

        std::size_t success = 0;
        std::size_t total = 0;

        auto recursively_bludgeon_dir = [&total, &success, &fixes](const std::filesystem::path &dir, auto &recursively_bludgeon_dir) -> void {
            for(auto i : std::filesystem::directory_iterator(dir)) {
                if(i.is_directory()) {
                    recursively_bludgeon_dir(i, recursively_bludgeon_dir);
                }
                else if(i.is_regular_file()) {
                    total++;
                    success += bludgeon_tag(i.path().string().c_str(), fixes) == EXIT_SUCCESS;
                }
            }
        };

        recursively_bludgeon_dir(std::filesystem::path(bludgeon_options.tags), recursively_bludgeon_dir);

        oprintf("Bludgeoned %zu out of %zu tag%s\n", success, total, total == 1 ? "" : "s");

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
            file_path = std::filesystem::path(bludgeon_options.tags) / Invader::File::halo_path_to_preferred_path(remaining_arguments[0]);
        }
        std::string file_path_str = file_path.string();
        return bludgeon_tag(file_path_str.c_str(), fixes);
    }
}
