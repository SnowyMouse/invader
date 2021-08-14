// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include <filesystem>
#include <invader/command_line_option.hpp>
#include <invader/version.hpp>
#include <invader/script/script_tree.hpp>
#include <invader/script/tokenizer.hpp>
#include <invader/script/compiler.hpp>
#include <invader/file/file.hpp>

int main(int argc, const char **argv) {
    Invader::setup_output();
    
    using namespace Invader;
    //using namespace Invader::HEK;

    struct ScriptOption {
        const char *path;
        const char *data = "data/";
        const char *tags = "tags/";
    } script_options;
    script_options.path = *argv;

    // Add our options
    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0);
    options.emplace_back("data", 'd', 1);
    options.emplace_back("tags", 't', 1);

    static constexpr char DESCRIPTION[] = "Compile scripts.";
    static constexpr char USAGE[] = "[options] <scenario>";

    // Parse arguments
    auto remaining_options = CommandLineOption::parse_arguments<ScriptOption &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, script_options, [](char opt, const auto &arguments, ScriptOption &script_options) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);

            case 'd':
                script_options.data = arguments[0];
                break;

            case 't':
                script_options.tags = arguments[0];
                break;
        }
    });

    // Get the scenario tag
    const char *scenario;
    if(remaining_options.size() == 0) {
        eprintf("Expected a scenario tag. Use -h for more information.\n");
        return EXIT_FAILURE;
    }
    else if(remaining_options.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_options[1]);
        return EXIT_FAILURE;
    }
    else {
        scenario = remaining_options[0];
    }

    // A simple function to clean tokens
    /*
        auto clean_token = [](const char *token) -> std::string {
        std::string s;
        for(const char *c = token; *c; c++) {
            if(*c == '\r') {
                s += "\\r";
            }
            else if(*c == '\n') {
                s += "\\n";
            }
            else if(*c == '\t') {
                s += "\\t";
            }
            else {
                s += *c;
            }
        }
        return s;
    };
    */

    std::filesystem::path tags(script_options.tags);
    std::filesystem::path data(script_options.data);

    std::filesystem::path tag_path = (tags / scenario);
    tag_path += std::string(".scenario");
    std::filesystem::path script_directory_path = (data / scenario).parent_path() / "scripts";

    // Make sure we have a scripts directory
    /*if(!std::filesystem::exists(script_directory_path)) {
        eprintf("Missing a scripts directory at %s\n", script_directory_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Put our nodes, scripts, and globals here
    std::vector<ScriptTree::Object> all_scripts;

    // Go through each script in the scripts directory
    for(auto &file : std::filesystem::directory_iterator(script_directory_path)) {
        auto &path = file.path();
        if(file.is_regular_file() && path.extension() == ".hsc") {
            auto path_str = path.string();
            auto file_data_maybe = Invader::File::open_file(path_str.c_str());
            if(!file_data_maybe.has_value()) {
                eprintf("Failed to open %s\n", path_str.c_str());
                return EXIT_FAILURE;
            }
            auto &file_data = file_data_maybe.value();

            // Add a 0 to the end for null termination
            file_data.push_back(static_cast<std::byte>(0));

            // Tokenize
            bool error;
            std::string error_message;
            std::size_t error_line = 0, error_column = 0;
            std::string error_token;
            auto tokens = Tokenizer::tokenize(reinterpret_cast<char *>(file_data.data()), error, error_line, error_column, error_token, error_message);

            // On failure, explain what happened
            if(error) {
                eprintf_error("Error parsing script %s", path_str.c_str());
                eprintf("%zu:%zu %s\n", error_line, error_column, clean_token(error_token.c_str()).c_str());
                eprintf("The error was: %s\n", error_message.c_str());
                return EXIT_FAILURE;
            }

            // Make a syntax tree
            auto tree = ScriptTree::compile_tokens(tokens, error, error_line, error_column, error_token, error_message);
            if(error) {
                eprintf_error("Error compiling script %s", path_str.c_str());
                eprintf("%zu:%zu %s\n", error_line, error_column, clean_token(error_token.c_str()).c_str());
                eprintf("The error was: %s\n", error_message.c_str());
                return EXIT_FAILURE;
            }

            all_scripts.insert(all_scripts.begin(), tree.begin(), tree.end());
        }
    }*/

    // Open the scenario tag
    auto tag_path_str = tag_path.string();
    auto scenario_file_data = Invader::File::open_file(tag_path_str.c_str());
    if(!scenario_file_data.has_value()) {
        eprintf("Failed to open %s\n", tag_path_str.c_str());
        return EXIT_FAILURE;
    }

    // Parse it
    Parser::Scenario s;
    try {
        s = Parser::Scenario::parse_hek_tag_file((*scenario_file_data).data(), (*scenario_file_data).size());
    }
    catch(std::exception &e) {
        eprintf("Failed to parse %s\n", tag_path_str.c_str());
        eprintf("The error was: %s\n", e.what());
        return EXIT_FAILURE;
    }

    auto new_s = Compiler::decompile_scenario(s);
    oprintf("%s\n\n", Tokenizer::detokenize(ScriptTree::decompile_script_tree(new_s)).c_str());
}
