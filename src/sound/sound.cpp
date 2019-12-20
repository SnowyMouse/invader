// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/version.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;

    struct SoundOptions {
        const char *data = "data";
        const char *tags = "tags";
        bool split = false;
    } sound_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("split", 's', 0, "Split permutations into 64 KiB chunks.");

    static constexpr char DESCRIPTION[] = "Create or modify a sound tag.";
    static constexpr char USAGE[] = "[options] <sound-tag>";

    auto remaining_arguments = CommandLineOption::parse_arguments<SoundOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, sound_options, [](char opt, const std::vector<const char *> &arguments, auto &sound_options) {
        switch(opt) {
            case 'd':
                sound_options.data = arguments[0];
                break;

            case 't':
                sound_options.tags = arguments[0];
                break;

            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);

            case 's':
                sound_options.split = true;
                eprintf("-s is not implemented at the moment...");
                break;
        }
    });

}
