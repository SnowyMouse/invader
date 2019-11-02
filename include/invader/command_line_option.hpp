// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__COMMAND_LINE_OPTION_HPP
#define INVADER__COMMAND_LINE_OPTION_HPP

#include <cstring>
#include <string>
#include <vector>
#include <optional>

namespace Invader {
    /**
     * Header only command line option handler
     */
    class CommandLineOption {
    private:
        /** Full name of the option */
        std::string full_name;

        /** Char name of the option */
        char char_name;

        /** Number of arguments expected */
        int argument_count;

        /** Description of the option */
        std::optional<std::string> description;

        /** Parameters of the option */
        std::optional<std::string> parameters;
    public:
        /**
         * Initialize a command line option
         * @param full_name      long name of the command line option
         * @param char_name      char name of the command line option; this will be passed into the argument handler if this is correctly used
         * @param argument_count number of arguments expected for the command line option
         * @param description    description of the command line option, if any
         * @param parameters     parameters of the command line option, if any
         */
        CommandLineOption(const char *full_name, const char char_name, int argument_count, const char *description = nullptr, const char *parameters = nullptr) : full_name(full_name), char_name(char_name), argument_count(argument_count), description(description ? std::optional<std::string>(std::string(description)) : std::nullopt), parameters(parameters ? std::optional<std::string>(std::string(parameters)) : std::nullopt) {}

        /**
         * Get the full name of the command line option
         * @return full name
         */
        const std::string &get_full_name() const noexcept {
            return this->full_name;
        }

        /**
         * Get the single char name of the command line option
         * @return char name
         */
        char get_char_name() const noexcept {
            return this->char_name;
        }

        /**
         * Get the argument count of the command line option
         * @return argument count
         */
        int get_argument_count() const noexcept {
            return this->argument_count;
        }

        /**
         * Parse the arguments using the given options with the first argument as the path to the program. When enough arguments are found for the given option, the char_name and all options found will be passed to handler. If an error occurs, error_char_name with 0 arguments will be passed.
         * @param argc            number of arguments
         * @param argv            pointer to the first argument
         * @param options         options to use
         * @param usage           program usage on help/error
         * @param description     program description on help
         * @param user_data       user data passed into the arguments
         * @param handler         handler function to use
         * @return                a vector of all unhandled arguments
         */
        template<typename DataType>
        static std::vector<const char *> parse_arguments(int argc, const char * const *argv, const std::vector<CommandLineOption> &options, const char *usage, const char *description, DataType user_data, void (*handler)(char char_name, const std::vector<const char *> &arguments, DataType user_data)) {
            // Hold all unhandled arguments
            std::vector<const char *> unhandled_arguments;

            // Hold the options to be handled
            std::vector<const CommandLineOption *> currently_handled_options;
            std::vector<const char *> currently_handled_option_arguments_handled;

            auto error_fail = [&argv, &usage](const char *what) {
                std::fprintf(stderr, "-%s is not a valid option.\n", what);
                std::fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
                std::fprintf(stderr, "Use -h for help.\n");
                std::exit(EXIT_FAILURE);
            };

            auto print_help = [&options, &argv, &usage, &description]() {
                // Print the usage and description
                std::printf("Usage: %s %s\n\n%s\n\nOptions:\n", argv[0], usage, description);

                auto print_help_for_option = [](const CommandLineOption &option) {
                    #define INV_COMMAND_LINE_ARGUMENT_SIZE 30
                    #define INV_COMMAND_LINE_ARGUMENT_MAKE_STR2(w) #w
                    #define INV_COMMAND_LINE_ARGUMENT_MAKE_STR(w) INV_COMMAND_LINE_ARGUMENT_MAKE_STR2(w)
                    #define INV_COMMAND_LINE_ARGUMENT_SIZE_STR INV_COMMAND_LINE_ARGUMENT_MAKE_STR(INV_COMMAND_LINE_ARGUMENT_SIZE)

                    // Print the left column (parameters and stuff)
                    char left_column[INV_COMMAND_LINE_ARGUMENT_SIZE];
                    std::size_t l = std::snprintf(left_column, sizeof(left_column), "  -%c --%s", option.char_name, option.full_name.data());
                    if(l >= sizeof(left_column)) {
                        std::fprintf(stderr, "Left column exceeded!\n");
                        std::terminate();
                    }
                    if(option.parameters.has_value()) {
                        l += std::snprintf(left_column + l, sizeof(left_column) - l, " %s", option.parameters.value().data());
                        if(l >= sizeof(left_column)) {
                            std::fprintf(stderr, "Left column exceeded!\n");
                            std::terminate();
                        }
                    }
                    std::printf("%-" INV_COMMAND_LINE_ARGUMENT_SIZE_STR "s", left_column);

                    if(!option.description.has_value()) {
                        std::printf("\n");
                        return;
                    }

                    const char *description = option.description.value().data();

                    // Print the right column (description)
                    std::size_t r = 0;
                    char right_column[80 - sizeof(left_column)] = {};
                    const char *word_start = nullptr;
                    bool first_line = true;

                    // Print and then purge the right column buffer to make way for a new line
                    auto purge_right_column = [&right_column, &r, &first_line, &word_start]() {
                        if(r) {
                            if(first_line) {
                                first_line = false;
                            }
                            else {
                                std::printf("%" INV_COMMAND_LINE_ARGUMENT_SIZE_STR "s", "");
                            }
                            std::printf("%s\n", right_column);
                            std::fill(right_column, right_column + sizeof(right_column), 0);
                            r = 0;
                        }
                    };

                    // Go through each word
                    for(const char *w = description;; w++) {
                        if(*w == ' ' || *w == 0) {
                            if(word_start) {
                                std::size_t word_length = w - word_start;
                                if(r + 1 + word_length >= sizeof(right_column)) {
                                    purge_right_column();
                                }
                                if(word_length + 1 > sizeof(right_column)) {
                                    std::fprintf(stderr, "Right column exceeded!\n");
                                    std::terminate();
                                }
                                right_column[r++] = ' ';
                                std::strncpy(right_column + r, word_start, word_length);
                                r += word_length;
                                word_start = nullptr;
                            }
                        }
                        else if(!word_start) {
                            word_start = w;
                        }

                        // If it's a null terminator, end the line and return
                        if(*w == 0) {
                            purge_right_column();
                            return;
                        }
                    }

                    #undef INV_COMMAND_LINE_ARGUMENT_SIZE
                    #undef INV_COMMAND_LINE_ARGUMENT_MAKE_STR2
                    #undef INV_COMMAND_LINE_ARGUMENT_MAKE_STR
                    #undef INV_COMMAND_LINE_ARGUMENT_SIZE_STR
                };

                auto print_help_for_option_letter = [&options, &print_help_for_option](char option_letter) {
                    // If option, show that
                    for(auto &option : options) {
                        if(option.char_name == option_letter) {
                            print_help_for_option(option);
                            return;
                        }
                    }
                    // If help, show that
                    if(option_letter == 'h') {
                        auto help_option = CommandLineOption("help", 'h', 0, "Show this list of options.");
                        print_help_for_option(help_option);
                        return;
                    }
                };

                for(char letter = 'a'; letter <= 'z'; letter++) {
                    print_help_for_option_letter(letter);
                    print_help_for_option_letter(std::toupper(letter));
                }

                std::printf("\n");

                std::exit(EXIT_SUCCESS);
            };

            for(int i = 1; i < argc; i++) {
                // If we don't have any options being worked on, check if this is an option
                if(currently_handled_options.size() == 0) {
                    // If it starts with a '-', then we've got something to do
                    const char *argument = argv[i];
                    std::size_t argument_length = std::strlen(argument);
                    if(argument_length >= 1 && argument[0] == '-') {
                        // It was just a hyphen
                        if(argument_length == 1) {
                            error_fail("");
                        }

                        // It's a long option
                        else if(argument[1] == '-') {
                            const char *long_option = argument + 2;
                            bool got_it = false;

                            // Find the option. If we have it, add it.
                            for(const auto &option : options) {
                                if(option.get_full_name() == long_option) {
                                    currently_handled_options.push_back(&option);
                                    got_it = true;
                                    break;
                                }
                            }

                            // If we didn't get it, then darn
                            if(!got_it) {
                                if(std::strcmp(argument, "--help") == 0) {
                                    print_help();
                                }
                                else {
                                    error_fail(argument + 1);
                                }
                            }
                        }

                        // It's a short option; possibly multiple of them
                        else {
                            for(const char *o = argument + 1; *o; o++) {
                                // Find the option. If we have it, add it.
                                bool got_it = false;
                                for(const auto &option : options) {
                                    if(option.get_char_name() == *o) {
                                        currently_handled_options.push_back(&option);
                                        got_it = true;
                                        break;
                                    }
                                }

                                // If we didn't get it, then darn
                                if(!got_it) {
                                    if(*o == 'h') {
                                        print_help();
                                    }
                                    else {
                                        char option_text[2] = { *o, 0 };
                                        error_fail(option_text);
                                    }
                                }
                            }

                            // Make sure only the last option takes arguments
                            for(std::size_t i = 0; i < currently_handled_options.size() - 1; i++) {
                                auto *option = currently_handled_options[i];
                                if(option->argument_count) {
                                    std::fprintf(stderr, "-%c (passed in \"%s\") takes %i argument%s.\n", option->char_name, argument, option->argument_count, option->argument_count == 1 ? "" : "s");
                                    std::fprintf(stderr, "Only the last option in an array of options can take arguments.\n");
                                    std::exit(EXIT_FAILURE);
                                }
                            }
                        }
                    }

                    // Otherwise, it wasn't a handled argument.
                    else {
                        unhandled_arguments.push_back(argv[i]);
                    }
                }

                // If we do have options being handled, add them to the handled list
                else {
                    currently_handled_option_arguments_handled.push_back(argv[i]);
                }

                // Check to see if we have any options that can be handled now
                while(currently_handled_options.size() > 0) {
                    auto &topmost_option = currently_handled_options[0];

                    // If we don't have enough arguments for the option, break and try again later
                    if(currently_handled_option_arguments_handled.size() < static_cast<std::size_t>(topmost_option->get_argument_count())) {
                        break;
                    }

                    // If we do, handle it
                    handler(topmost_option->get_char_name(), currently_handled_option_arguments_handled, user_data);

                    // Clear the array up
                    currently_handled_option_arguments_handled.clear();

                    // Remove the first item
                    currently_handled_options.erase(currently_handled_options.begin());
                }
            }

            // If we have unfinished business, do it
            std::size_t remaining_options = currently_handled_options.size();
            if(remaining_options != 0) {
                auto &arg = currently_handled_options[0];
                int expected_argument_count = arg->get_argument_count();
                std::size_t remaining_argument_count = currently_handled_option_arguments_handled.size();
                std::fprintf(stderr, "-%c takes %i argument%s, but only %zu %s given.\n", arg->get_char_name(), expected_argument_count, expected_argument_count == 1 ? "" : "s", remaining_argument_count, remaining_argument_count == 1 ? "was" : "were");
                std::exit(EXIT_FAILURE);
            }

            return unhandled_arguments;
        }
    };
}

#endif
