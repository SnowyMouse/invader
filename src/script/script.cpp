// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include <filesystem>
#include "../eprintf.hpp"
#include "../command_line_option.hpp"
#include "../version.hpp"
#include "tokenizer.hpp"

int main(int argc, const char **argv) {
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
    options.emplace_back("help", 'h', 0);
    options.emplace_back("info", 'i', 0);
    options.emplace_back("data", 'd', 1);
    options.emplace_back("tags", 't', 1);

    // Parse arguments
    auto remaining_options = CommandLineOption::parse_arguments<ScriptOption &>(argc, argv, options, 'h', script_options, [](char opt, const auto &arguments, ScriptOption &script_options) {
        switch(opt) {
            case 'i':
                INVADER_SHOW_INFO
                std::exit(EXIT_FAILURE);
                break;

            case 'd':
                script_options.data = arguments[0];
                break;

            case 't':
                script_options.tags = arguments[0];
                break;

            default:
                eprintf("Usage: %s <options> <scenario>\n", script_options.path);
                eprintf("Compile scripts for scenario tags.\n\n");
                eprintf("Options:\n");
                eprintf("  --help,-h                    Show this help directory.\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --data,-d <dir>              Use a specific data directory.\n");
                eprintf("  --tags,-t <dir>              Use a specific tags directory.\n\n");
                std::exit(EXIT_FAILURE);
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

    std::filesystem::path tags(script_options.tags);
    std::filesystem::path data(script_options.data);

    std::filesystem::path tag_path = tags / scenario;
    std::filesystem::path script_directory_path = data / scenario / "scripts";

    // Make sure we have a scripts directory
    if(!std::filesystem::exists(script_directory_path)) {
        eprintf("Missing a scripts directory at %s\n", script_directory_path.string().data());
        return EXIT_FAILURE;
    }

    // Go through each script in the scripts directory
    for(auto &file : std::filesystem::directory_iterator(script_directory_path)) {
        auto &path = file.path();
        if(file.is_regular_file() && path.extension() == ".hsc") {
            // Read the file data
            auto path_str = path.string();
            std::FILE *f = std::fopen(path_str.data(), "rb");
            if(!f) {
                eprintf("Failed to open %s\n", path_str.data());
                return EXIT_FAILURE;
            }
            std::fseek(f, 0, SEEK_END);
            std::vector<char> file_data(std::ftell(f));
            std::fseek(f, 0, SEEK_SET);
            if(std::fread(file_data.data(), file_data.size(), 1, f) != 1) {
                eprintf("Failed to read %s\n", path_str.data());
                std::fclose(f);
                return EXIT_FAILURE;
            }
            std::fclose(f);

            // Add a 0 to the end for null termination
            file_data.push_back(0);

            // Tokenize
            bool error;
            std::size_t error_line, error_column;
            std::string error_token;
            auto tokens = Invader::Tokenizer::tokenize(file_data.data(), error, error_line, error_column, error_token);

            // On failure, explain what happened
            if(error) {
                eprintf("Error parsing %s at %zu:%zu in script %s\n", clean_token(error_token.data()).data(), error_line, error_column, path_str.data());
                return EXIT_FAILURE;
            }

            char line_str[256];

            for(auto &t : tokens) {
                std::snprintf(line_str, sizeof(line_str), "%zu:%zu", t.line, t.column);

                switch(t.type) {
                    case Tokenizer::TokenType::TOKEN_TYPE_STRING:
                        eprintf("%-8sSTRING:  \"%s\"\n", line_str, clean_token(std::any_cast<std::string>(t.value).data()).data());
                        break;
                    case Tokenizer::TokenType::TOKEN_TYPE_DECIMAL:
                        eprintf("%-8sDECIMAL: %f\n", line_str, std::any_cast<float>(t.value));
                        break;
                    case Tokenizer::TokenType::TOKEN_TYPE_INTEGER:
                        eprintf("%-8sINTEGER: %i\n", line_str, std::any_cast<std::int32_t>(t.value));
                        break;
                    case Tokenizer::TokenType::TOKEN_TYPE_PARENTHESIS_OPEN:
                        eprintf("%-8sPARENTHESIS_OPEN\n", line_str);
                        break;
                    case Tokenizer::TokenType::TOKEN_TYPE_PARENTHESIS_CLOSE:
                        eprintf("%-8sPARENTHESIS_CLOSE\n", line_str);
                        break;
                }
            }
        }
    }
}
