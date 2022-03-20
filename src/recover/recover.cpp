// SPDX-License-Identifier: GPL-3.0-only

#include "../command_line_option.hpp"
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include "recover_method.hpp"

int main(int argc, const char **argv) {
    set_up_color_term();
    
    using namespace Invader;
    using namespace Invader::HEK;
    
    struct RecoverOptions {
        std::filesystem::path tags = "tags";
        std::filesystem::path data = "data";
        bool overwrite = false;
        std::vector<std::string> batch, batch_exclude;
    } recover_options;

    const CommandLineOption options[] = {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_DATA),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_BATCH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_BATCH_EXCLUDE),
        CommandLineOption("overwrite", 'O', 0, "Overwrite data if it already exists"),
    };

    static constexpr char DESCRIPTION[] = "Recover source data from tags.";
    static constexpr char USAGE[] = "[options] <-b <expr> | <tag.group>>";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<RecoverOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 1, recover_options, [](char opt, const auto &args, RecoverOptions &recover_options) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'd':
                recover_options.data = args[0];
                break;
            case 'O':
                recover_options.overwrite = true;
                break;
            case 't':
                recover_options.tags = args[0];
                break;
            case 'b':
                recover_options.batch.emplace_back(args[0]);
                break;
            case 'e':
                recover_options.batch_exclude.emplace_back(args[0]);
                break;
        }
    });
    
    if(!std::filesystem::is_directory(recover_options.data)) {
        eprintf_error("Data folder %s does not exist or is not a valid directory", recover_options.data.string().c_str());
        return EXIT_FAILURE;
    }
    
    auto uses_batching = !(recover_options.batch.empty() && recover_options.batch_exclude.empty());
    if(uses_batching != remaining_arguments.empty()) {
        eprintf_error("Expected a tag path OR batching (not both)");
        return EXIT_FAILURE;
    }
    
    bool result = false;
    
    auto do_on_tag = [&recover_options, &result](const auto &tag) -> bool {
        // read it
        auto file_path = File::tag_path_to_file_path(tag, recover_options.tags);
        auto file = File::open_file(file_path);
        if(!file.has_value()) {
            eprintf_error("Failed to read %s", file_path.string().c_str());
            return false;
        }
        
        // Load it
        auto tag_data = Parser::ParserStruct::parse_hek_tag_file(file->data(), file->size());
        auto r = Recover::recover(*tag_data, std::filesystem::path(tag).replace_extension().string(), recover_options.data, reinterpret_cast<const HEK::TagFileHeader *>(file->data())->tag_fourcc, recover_options.overwrite);
        result = r || result;
        return r;
    };
    
    // Let's do this
    if(uses_batching) {
        auto virtual_tags = File::load_virtual_tag_folder({recover_options.tags});
        std::size_t total = 0;
        std::size_t recovered = 0;
        for(auto &t : virtual_tags) {
            if(File::path_matches(t.tag_path.c_str(), recover_options.batch, recover_options.batch_exclude)) {
                total++;
                if(!do_on_tag(t.tag_path)) {
                    eprintf("Skipped %s\n", t.tag_path.c_str());
                }
                else {
                    oprintf_success("Recovered %s", t.tag_path.c_str());
                    recovered++;
                }
            }
        }
        oprintf("Recovered %zu of %zu tag%s\n", recovered, total, total == 1 ? "" : "s");
    }
    else {
        do_on_tag(File::halo_path_to_preferred_path(remaining_arguments[0]));
        if(result) {
            oprintf_success("Recovered %s", remaining_arguments[0]);
        }
        else {
            eprintf_error("Could not recover %s", remaining_arguments[0]);
        }
    }
    
    if(result) {
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}
